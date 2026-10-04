package com.obsifox.generals.net

import android.os.Handler
import android.os.Looper
import android.util.Log
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.OutputStreamWriter
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.net.InetSocketAddress
import java.net.ServerSocket
import java.net.Socket
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.atomic.AtomicBoolean

/**
 * LAN host: TCP session server + UDP discovery announcer.
 * Threading: one accept loop, one reader per client, one announcer thread.
 * All UI callbacks are marshalled to the main thread.
 */
class LanHost(
    private val hostNick: String,
    private val gameName: String,
    private val listener: Listener
) {

    interface Listener {
        fun onRosterChanged(players: List<JSONObject>)
        fun onChat(from: String, msg: String)
        fun onError(message: String)
    }

    private val running = AtomicBoolean(false)
    private val main = Handler(Looper.getMainLooper())

    // nick -> connection
    private val clients = ConcurrentHashMap<String, ClientConn>()
    private val readyState = ConcurrentHashMap<String, Boolean>()

    private var serverSocket: ServerSocket? = null
    private var announceSocket: DatagramSocket? = null
    private var announceThread: Thread? = null
    private var acceptThread: Thread? = null

    fun start() {
        if (!running.compareAndSet(false, true)) return
        Thread({
            try {
                serverSocket = ServerSocket()
                serverSocket?.bind(InetSocketAddress(LanProtocol.TCP_SESSION_PORT))
                announceSocket = DatagramSocket().apply { broadcast = true }
                startAnnouncer()
                startAcceptLoop()
            } catch (t: Throwable) {
                main.post { listener.onError(t.message ?: "bind failed") }
                stop()
            }
        }, "lan-host").start()
    }

    private fun startAcceptLoop() {
        acceptThread = Thread({
            while (running.get()) {
                try {
                    val s = serverSocket?.accept() ?: break
                    handleNewClient(s)
                } catch (t: Throwable) {
                    if (running.get()) main.post { listener.onError(t.message ?: "accept") }
                }
            }
        }, "lan-host-accept").start()
    }

    private fun handleNewClient(socket: Socket) {
        val reader = BufferedReader(InputStreamReader(socket.getInputStream(), Charsets.UTF_8))
        val writer = OutputStreamWriter(socket.getOutputStream(), Charsets.UTF_8)
        val first = reader.readLine() ?: return closeQuietly(socket)
        val hello = LanProtocol.parse(first) ?: return closeQuietly(socket)
        if (hello.optString("t") != "hello") return closeQuietly(socket)
        val nick = hello.optString("nick").ifBlank { "Player" }.take(24)

        if (clients.size >= LanProtocol.MAX_PLAYERS - 1) {
            writer.write(LanProtocol.joinOk(false, "full")); writer.flush()
            return closeQuietly(socket)
        }
        if (clients.containsKey(nick)) {
            writer.write(LanProtocol.joinOk(false, "name-taken")); writer.flush()
            return closeQuietly(socket)
        }

        writer.write(LanProtocol.joinOk(true)); writer.flush()
        val conn = ClientConn(nick, socket, reader, writer)
        clients[nick] = conn
        readyState[nick] = false
        broadcastRoster()
        main.post { listener.onChat("", "$nick@join") } // coded events, UI maps to localized text
        Thread({
            conn.readLoop()
        }, "lan-host-$nick").start()
    }

    private inner class ClientConn(
        val nick: String,
        val socket: Socket,
        private val reader: BufferedReader,
        private val writer: OutputStreamWriter
    ) {
        @Volatile var alive = true

        fun readLoop() {
            try {
                while (running.get() && alive) {
                    val line = reader.readLine() ?: break
                    val msg = LanProtocol.parse(line) ?: continue
                    when (msg.optString("t")) {
                        "chat" -> {
                            val m = msg.optString("msg")
                            main.post { listener.onChat(nick, m) }
                            sendToAll(LanProtocol.chat(nick, m), except = null)
                        }
                        "ready" -> {
                            readyState[nick] = msg.optBoolean("ready", false)
                            broadcastRoster()
                        }
                        "ping" -> send(LanProtocol.ping())
                    }
                }
            } catch (t: Throwable) { /* normal on disconnect */ } finally {
                alive = false
                clients.remove(nick)
                readyState.remove(nick)
                closeQuietly(socket)
                if (running.get()) {
                    broadcastRoster()
                    main.post { listener.onChat("", "$nick@leave") }
                }
            }
        }

        fun send(payload: String): Boolean = try {
            synchronized(writer) { writer.write(payload); writer.flush() }
            true
        } catch (t: Throwable) { alive = false; false }
    }

    private fun broadcastRoster() {
        val players = buildList {
            add(LanProtocol.player(hostNick, true, host = true))
            clients.values.forEach { add(LanProtocol.player(it.nick, readyState[it.nick] ?: false, host = false)) }
        }
        main.post { listener.onRosterChanged(players) }
        sendToAll(LanProtocol.roster(players), except = null)
    }

    private fun sendToAll(payload: String, except: String?) {
        for (c in clients.values) {
            if (except != null && c.nick == except) continue
            c.send(payload)
        }
    }

    fun sendChat(msg: String) {
        sendToAll(LanProtocol.chat(hostNick, msg), except = null)
    }

    fun startGame() { sendToAll(LanProtocol.start(), except = null) }

    private fun startAnnouncer() {
        announceThread = Thread({
            val buf = LanProtocol.announce(gameName, clients.size + 1, hostNick).toByteArray(Charsets.UTF_8)
            while (running.get()) {
                try {
                    val addr = InetAddress.getByName("255.255.255.255")
                    announceSocket?.send(DatagramPacket(buf, buf.size, addr, LanProtocol.UDP_DISCOVERY_PORT))
                    // subnet-directed fallback for common Android hotspot subnet
                    val hot = InetAddress.getByName("192.168.43.255")
                    announceSocket?.send(DatagramPacket(buf, buf.size, hot, LanProtocol.UDP_DISCOVERY_PORT))
                } catch (t: Throwable) {
                    Log.w("LanHost", "announce: ${t.message}")
                }
                try { Thread.sleep(2000) } catch (t: InterruptedException) { break }
            }
        }, "lan-host-announce").start()
    }

    fun stop() {
        if (!running.compareAndSet(true, false)) return
        try { serverSocket?.close() } catch (t: Throwable) {}
        try { announceSocket?.close() } catch (t: Throwable) {}
        for (c in clients.values) { c.alive = false; closeQuietly(c.socket) }
        clients.clear()
        announceThread?.interrupt()
    }

    private fun closeQuietly(s: Socket) { try { s.close() } catch (t: Throwable) {} }
}
