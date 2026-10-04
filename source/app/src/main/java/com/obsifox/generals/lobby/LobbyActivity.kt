package com.obsifox.generals.lobby

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.text.InputType
import android.view.Gravity
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import com.obsifox.generals.GeneralsApp
import com.obsifox.generals.R
import com.obsifox.generals.net.HotspotNet
import com.obsifox.generals.net.LanClient
import com.obsifox.generals.net.LanHost
import com.obsifox.generals.net.LanProtocol
import com.obsifox.generals.util.Persian
import com.obsifox.generals.util.Ui
import org.json.JSONObject

/**
 * Real hotspot LAN lobby (docs/HOTSPOT-MULTIPLAYER.md).
 * Works between two phones TODAY, independently of the game engine.
 * Start button honestly reports "waiting for engine link (phase 16)".
 */
class LobbyActivity : AppCompatActivity(), LanHost.Listener {

    private lateinit var modeLine: TextView
    private lateinit var playersBox: LinearLayout
    private lateinit var foundBox: LinearLayout
    private lateinit var chatLog: TextView
    private lateinit var chatInput: EditText
    private lateinit var hostBtn: Button
    private lateinit var joinScanBtn: Button
    private lateinit var readyBtn: Button
    private lateinit var startBtn: Button
    private lateinit var leaveBtn: Button

    private var host: LanHost? = null
    private var client: LanClient? = null
    private var amReady = false
    private var isHosting = false

    private val main = Handler(Looper.getMainLooper())
    private val nick: String by lazy {
        val p = Persian.prefs(this)
        p.getString(GeneralsApp.KEY_NICK, null) ?: ("Player-" + (1000..9999).random()).also {
            p.edit().putString(GeneralsApp.KEY_NICK, it).apply()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.statusBarColor = getColor(R.color.status_bar)
        buildUi()
    }

    private fun buildUi() {
        val root = ScrollView(this)
        val box = Ui.linear(this)
        box.setPadding(Ui.dp(box, 18), Ui.dp(box, 24), Ui.dp(box, 18), Ui.dp(box, 18))
        root.addView(box, ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))

        val title = Ui.centerTitle(this, getString(R.string.lobby_title))
        box.addView(title)
        val hint = Ui.label(this, getString(R.string.lobby_hotspot_hint), 12,
            color = R.color.text_secondary)
        hint.gravity = Gravity.CENTER
        box.addView(hint); Ui.margins(hint, top = 4, bottom = 14)

        modeLine = Ui.label(this, "", 14, bold = true, color = R.color.accent)
        box.addView(modeLine); Ui.margins(modeLine, bottom = 10)

        hostBtn = Ui.menuButton(this, getString(R.string.lobby_host_btn)) { startHosting() }
        box.addView(hostBtn)
        joinScanBtn = Ui.menuButton(this, getString(R.string.lobby_join_btn)) { startScanning() }
        box.addView(joinScanBtn)

        box.addView(Ui.label(this, getString(R.string.lobby_discovered), 14, bold = true))
        Ui.margins(box.getChildAt(box.childCount - 1), top = 14)
        foundBox = Ui.linear(this)
        box.addView(foundBox)
        box.addView(Ui.label(this, getString(R.string.lobby_no_games), 12,
            color = R.color.text_secondary).also { foundTag = it })

        box.addView(Ui.label(this, getString(R.string.lobby_players, 0), 14, bold = true))
        Ui.margins(box.getChildAt(box.childCount - 1), top = 14)
        playersBox = Ui.linear(this)
        playersBox.background = getDrawable(R.drawable.bg_card)
        playersBox.setPadding(Ui.dp(playersBox, 12), Ui.dp(playersBox, 10),
            Ui.dp(playersBox, 12), Ui.dp(playersBox, 10))
        box.addView(playersBox); Ui.margins(playersBox, top = 6, bottom = 12)

        val actions = Ui.linear(this, vertical = false)
        readyBtn = Ui.menuButton(this, getString(R.string.lobby_ready_toggle)) {
            amReady = !amReady
            client?.sendReady(amReady)
        }
        startBtn = Ui.menuButton(this, getString(R.string.lobby_start)) { startGame() }
        actions.addView(readyBtn); Ui.margins(readyBtn, right = 6)
        actions.addView(startBtn); Ui.margins(startBtn, left = 6)
        box.addView(actions)

        leaveBtn = Ui.menuButton(this, getString(R.string.lobby_leave_btn)) { stopAll("leave") }
        box.addView(leaveBtn)

        box.addView(Ui.label(this, "Chat", 14, bold = true))
        Ui.margins(box.getChildAt(box.childCount - 1), top = 14)
        chatLog = Ui.label(this, "", 13, color = R.color.text_primary)
        chatLog.background = getDrawable(R.drawable.bg_card)
        chatLog.setPadding(Ui.dp(chatLog, 12), Ui.dp(chatLog, 10),
            Ui.dp(chatLog, 12), Ui.dp(chatLog, 10))
        chatLog.minHeight = Ui.dp(chatLog, 140)
        box.addView(chatLog)

        val chatRow = Ui.linear(this, vertical = false)
        chatInput = Ui.input(this, getString(R.string.lobby_chat_hint))
        chatInput.inputType = InputType.TYPE_CLASS_TEXT
        chatInput.maxLines = 1
        chatRow.addView(chatInput, LinearLayout.LayoutParams(0,
            ViewGroup.LayoutParams.WRAP_CONTENT, 1f))
        val send = Ui.menuButton(this, getString(R.string.lobby_send)) { sendChat() }
        chatRow.addView(send, LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT))
        box.addView(chatRow); Ui.margins(chatRow, top = 8)

        setContentView(root)
        setModeIdle()
    }

    private var foundTag: TextView? = null

    // ————— host —————

    private fun startHosting() {
        stopAll("restart")
        HotspotNet.acquire(this)
        isHosting = true
        host = LanHost(nick, gameName(), this).also {
            it.start()
        }
        setModeHosting()
    }

    private fun gameName(): String {
        val p = Persian.prefs(this)
        return p.getString(GeneralsApp.KEY_GAME_NAME, null)
            ?: getString(R.string.lobby_default_game)
    }

    private fun setModeHosting() {
        modeLine.text = "▸ ${getString(R.string.lobby_host_btn)} — $nick"
        hostBtn.isEnabled = false
        joinScanBtn.isEnabled = false
        leaveBtn.isEnabled = true
        readyBtn.visibility = TextView.GONE
        startBtn.visibility = TextView.VISIBLE
        foundTag?.visibility = TextView.GONE
        foundBox.removeAllViews()
    }

    // ————— client —————

    private fun startScanning() {
        stopAll("restart")
        HotspotNet.acquire(this)
        isHosting = false
        foundBox.removeAllViews()
        foundTag?.text = getString(R.string.lobby_scanning)
        foundTag?.visibility = TextView.VISIBLE
        client = LanClient(nick, object : LanClient.Listener {
            override fun onGameFound(info: LanClient.GameInfo) {
                main.post { addFoundGame(info) }
            }
            override fun onRosterChanged(players: List<JSONObject>) {
                main.post { renderRoster(players) }
            }
            override fun onChat(from: String, msg: String) {
                main.post { appendChat(from, msg) }
            }
            override fun onStartRequested() {
                main.post { showEngineWait() }
            }
            override fun onConnected(gameName: String) {
                main.post { modeLine.text = getString(R.string.lobby_connected_to, gameName) }
            }
            override fun onDisconnected(reason: String) {
                main.post {
                    appendChat("•", getString(R.string.lobby_disconnected))
                    setModeIdle()
                }
            }
            override fun onError(message: String) {
                main.post { appendChat("✕", message) }
            }
        }).also {
            it.startDiscovery()
        }
        joinByIpDialog()
    }

    private fun addFoundGame(info: LanClient.GameInfo) {
        foundTag?.visibility = TextView.GONE
        // dedupe by address
        for (i in 0 until foundBox.childCount) {
            val tag = foundBox.getChildAt(i).tag as? String ?: continue
            if (tag == info.address) return
        }
        val b = Ui.menuButton(this,
            "${info.gameName} — ${info.hostNick} (${Persian.num(this, info.players)}/${Persian.num(this, info.maxPlayers)})"
        ) { client?.connect(info.address) }
        b.tag = info.address
        foundBox.addView(b)
    }

    private fun joinByIpDialog() {
        val input = Ui.input(this, getString(R.string.lobby_ip_hint))
        input.inputType = InputType.TYPE_CLASS_PHONE
        AlertDialog.Builder(this)
            .setTitle(getString(R.string.lobby_join_by_ip))
            .setView(input)
            .setPositiveButton(getString(R.string.lobby_join)) { _, _ ->
                val ip = input.text.toString().trim()
                if (ip.isNotEmpty()) client?.connect(ip)
            }
            .setNegativeButton(getString(R.string.cancel), null)
            .show()
    }

    // ————— shared —————

    private fun renderRoster(players: List<JSONObject>) {
        playersBox.removeAllViews()
        playersBox.addView(Ui.label(this,
            getString(R.string.lobby_players, players.size), 14, bold = true))
        for (p in players) {
            val nickP = p.optString("nick", "?")
            val hostP = p.optBoolean("host", false)
            val readyP = p.optBoolean("ready", false)
            val line = buildString {
                append((if (readyP) "● " else "○ "))
                append(nickP)
                if (hostP) append(" " + getString(R.string.lobby_host_is, "").trim())
            }
            playersBox.addView(Ui.label(this, line, 13,
                color = if (readyP) R.color.ok_green else R.color.text_primary))
        }
        if (isHosting && players.size >= 2) {
            // host reflects clients' ready state via roster broadcasts
        }
    }

    private fun startGame() {
        if (isHosting) host?.startGame()
        showEngineWait()
    }

    private fun showEngineWait() {
        AlertDialog.Builder(this)
            .setTitle(getString(R.string.lobby_start))
            .setMessage(getString(R.string.lobby_waiting_engine))
            .setPositiveButton(getString(R.string.ok), null)
            .show()
    }

    private fun sendChat() {
        val msg = chatInput.text.toString().trim()
        if (msg.isEmpty()) return
        if (isHosting) host?.sendChat(msg) else client?.sendChat(msg)
        chatInput.setText("")
    }

    private fun setModeIdle() {
        modeLine.text = ""
        hostBtn.isEnabled = true
        joinScanBtn.isEnabled = true
        readyBtn.visibility = TextView.GONE
        startBtn.visibility = TextView.GONE
        foundBox.removeAllViews()
        foundTag?.visibility = TextView.GONE
    }

    // ————— LanHost.Listener —————

    override fun onRosterChanged(players: List<JSONObject>) {
        main.post { renderRoster(players) }
    }

    override fun onChat(from: String, msg: String) {
        main.post { appendChat(from, msg) }
    }

    override fun onError(message: String) {
        main.post { appendChat("✕", message) }
    }

    private fun appendChat(from: String, msg: String) {
        val line = when {
            from.isEmpty() -> translateEvent(msg)
            msg == "join" -> getString(R.string.lobby_player_joined, from)
            msg == "leave" -> getString(R.string.lobby_player_left, from)
            else -> "$from: $msg"
        }
        chatLog.append(line + "\n")
    }

    private fun translateEvent(msg: String): String = when {
        msg.endsWith("@join") -> getString(R.string.lobby_player_joined, msg.removeSuffix("@join"))
        msg.endsWith("@leave") -> getString(R.string.lobby_player_left, msg.removeSuffix("@leave"))
        else -> msg
    }

    private fun stopAll(reason: String) {
        try { host?.stop() } catch (t: Throwable) {}
        try { client?.stop() } catch (t: Throwable) {}
        host = null
        client = null
        HotspotNet.release()
        if (reason != "leave") setModeIdle() else finish()
    }

    override fun onDestroy() {
        stopAll("destroy")
        super.onDestroy()
    }
}
