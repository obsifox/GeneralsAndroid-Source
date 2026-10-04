package com.obsifox.generals.io

import android.content.Context

/**
 * Central store for imported game-data location (Master Prompt §13).
 * The SAF tree URI is persisted (takePersistableUriPermission) so access
 * survives reboot without any fixed filesystem path assumption.
 */
object GameDataStore {

    fun saveGameData(context: Context, treeUri: String, gameType: String) {
        val p = com.obsifox.generals.util.Persian.prefs(context)
        p.edit()
            .putString(com.obsifox.generals.GeneralsApp.KEY_GAME_URI, treeUri)
            .putString(com.obsifox.generals.GeneralsApp.KEY_GAME_TYPE, gameType)
            .putBoolean(com.obsifox.generals.GeneralsApp.KEY_WIZARD_DONE, true)
            .apply()
    }

    fun gameDataUri(context: Context): String? =
        com.obsifox.generals.util.Persian.prefs(context)
            .getString(com.obsifox.generals.GeneralsApp.KEY_GAME_URI, null)

    fun gameType(context: Context): String =
        com.obsifox.generals.util.Persian.prefs(context)
            .getString(com.obsifox.generals.GeneralsApp.KEY_GAME_TYPE, "zh") ?: "zh"

    fun isConfigured(context: Context): Boolean = !gameDataUri(context).isNullOrBlank()
}
