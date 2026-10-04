package com.obsifox.generals.net

import org.json.JSONArray
import org.json.JSONObject

/**
 * LAN lobby protocol (docs/HOTSPOT-MULTIPLAYER.md).
 * Newline-delimited JSON over TCP 45778; discovery broadcast JSON over UDP 45777.
 * Deliberately engine-independent (Master Prompt §20): later phases bridge this
 * lobby to the engine's own LAN networking.
 */
object LanProtocol {

    const val UDP_DISCOVERY_PORT = 45777
    const val TCP_SESSION_PORT = 45778
    const val PROTOCOL_VERSION = 1
    const val MAX_PLAYERS = 8

    private const val NL = '\n'

    fun announce(gameName: String, players: Int, hostNick: String): String = JSONObject()
        .put("t", "announce")
        .put("v", PROTOCOL_VERSION)
        .put("name", gameName)
        .put("players", players)
        .put("max", MAX_PLAYERS)
        .put("host", hostNick)
        .toString() + NL

    fun hello(nick: String): String = JSONObject()
        .put("t", "hello")
        .put("v", PROTOCOL_VERSION)
        .put("nick", nick)
        .toString() + NL

    fun roster(players: List<JSONObject>): String = JSONObject()
        .put("t", "roster")
        .put("players", JSONArray(players))
        .toString() + NL

    fun player(nick: String, ready: Boolean, host: Boolean): JSONObject = JSONObject()
        .put("nick", nick)
        .put("ready", ready)
        .put("host", host)

    fun chat(from: String, msg: String): String = JSONObject()
        .put("t", "chat")
        .put("from", from)
        .put("msg", msg.take(300))
        .toString() + NL

    fun ready(who: String, ready: Boolean): String = JSONObject()
        .put("t", "ready")
        .put("who", who)
        .put("ready", ready)
        .toString() + NL

    fun joinOk(ok: Boolean, reason: String = ""): String = JSONObject()
        .put("t", "joinok")
        .put("ok", ok)
        .put("reason", reason)
        .toString() + NL

    fun ping(): String = JSONObject().put("t", "ping").toString() + NL

    fun start(): String = JSONObject().put("t", "start").toString() + NL

    fun parse(line: String): JSONObject? = try {
        JSONObject(line.trim())
    } catch (t: Throwable) {
        null
    }
}
