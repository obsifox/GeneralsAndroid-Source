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
import java.net.Socket
import java.util.concurrent.atomic.AtomicBoolean

/**
 * LAN client: UDP discovery listener + TCP session joiner.
 * Discovery binds UDP 45777 with SO_REUSEADDR so several phones on the same
 * hotspot can listen simultaneously to the host's broadcast announcements.
 */
class LanClient(
    private val nick: String,
    private val listener: Listener
) {

    interface Listener {
        fun onGameFound(info: GameInfo)
        fun onRosterChanged(players: List<JSONObject>)
        fun onChat(from: String, msg: String)
        fun onStartRequested()
        fun onConnected(gameName: String)
        fun onDisconnected(reason: String)
        fun onError(message: String)
    }

    data class GameInfo(
        val address: String,
        val gameName: String,
        val players: Int,
        val maxPlayers: Int,
        val hostNick: String,
        val protocolVersion: Int
    )

    private val running = AtomicBoolean(false)
    private val main = Handler(Looper.getMainLooper())

    private var discoverySocket: DatagramSocket? = null
    private var discoveryThread: Thread? = null

    private var socket: Socket? = null
    private var writer: OutputStreamWriter? = null
    private var readerThread: Thread? = null

    fun startDiscovery() {
        if (!running.compareAndSet(false, true)) return
        Thread({
            try {
                discoverySocket = DatagramSocket(null).apply {
                    reuseAddress = true
                    bind(InetSocketAddress(LanProtocol.UDP_DISCOVERY_PORT))
                    broadcast = true
                }
                val buf = ByteArray(2048)
                while (running.get()) {
                    val packet = DatagramPacket(buf, buf.size)
                    discoverySocket?.receive(packet) ?: break
                    val msg = LanProtocol.parse(String(packet.data, 0, packet.length, Charsets.UTF_8))
                    if (msg?.optString("t") == "announce" &&
                        msg.optInt("v") == LanProtocol.PROTOCOL_VERSION) {
                        val info = GameInfo(
                            address = packet.address.hostAddress ?: continue,
                            gameName = msg.optString("name", "?"),
                            players = msg.optInt("players", 1),
                            maxPlayers = msg.optInt("max", 8),
                            hostNick = msg.optString("host", "?"),
                            protocolVersion = msg.optInt("v", 0)
                        )
                        main.post { listener.onGameFound(info) }
                    }
                }
            } catch (t: Throwable) {
                if (running.get()) Log.w("LanClient", "discovery: ${t.message}")
            }
        }, "lan-client-discovery").also { discoveryThread = it; it.start() }
    }

    fun connect(ip: String) {
        Thread({
            try {
                val s = Socket()
                s.connect(InetSocketAddress(ip, LanProtocol.TCP_SESSION_PORT), 5000)
                socket = s
                val w = OutputStreamWriter(s.getOutputStream(), Charsets.UTF_8)
                writer = w
                w.write(LanProtocol.hello(nick)); w.flush()
                val r = BufferedReader(InputStreamReader(s.getInputStream(), Charsets.UTF_8))
                while (running.get()) {
                    val line = r.readLine() ?: break
                    val msg = LanProtocol.parse(line) ?: continue
                    when (msg.optString("t")) {
                        "joinok" -> {
                            if (!msg.optBoolean("ok", false)) {
                                main.post { listener.onDisconnected(msg.optString("reason", "denied")) }
                                return@Thread
                            } else {
                                main.post { listener.onConnected(ip) }
                            }
                        }
                        "roster" -> {
                            val arr = msg.optJSONArray("players")
                            val list = mutableListOf<JSONObject>()
                            if (arr != null) for (i in 0 until arr.length()) list.add(arr.getJSONObject(i))
                            main.post { listener.onRosterChanged(list) }
                        }
                        "chat" -> main.post { listener.onChat(msg.optString("from"), msg.optString("msg")) }
                        "start" -> main.post { listener.onStartRequested() }
                        "ping" -> send(LanProtocol.ping())
                    }
                }
                if (running.get()) main.post { listener.onDisconnected("eof") }
            } catch (t: Throwable) {
                main.post { listener.onError(t.message ?: "connect failed") }
            }
        }, "lan-client-session").also { readerThread = it; it.start() }
    }

    fun send(payload: String): Boolean = try {
        val w = writer ?: return false
        synchronized(w) { w.write(payload); w.flush() }
        true
    } catch (t: Throwable) { false }

    fun sendChat(msg: String) = send(LanProtocol.chat(nick, msg))
    fun sendReady(ready: Boolean) = send(LanProtocol.ready(nick, ready))

    fun stop() {
        if (!running.compareAndSet(true, false)) return
        try { discoverySocket?.close() } catch (t: Throwable) {}
        try { socket?.close() } catch (t: Throwable) {}
        discoveryThread?.interrupt()
    }
}
