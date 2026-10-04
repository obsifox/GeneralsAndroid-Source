package com.obsifox.generals

import android.app.Application
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.os.LocaleListCompat

/**
 * Application entry — per-app locale (fa/en) is applied here so every activity
 * is RTL-correct without duplicating logic (Master Prompt §9, §11).
 */
class GeneralsApp : Application() {

    override fun onCreate() {
        super.onCreate()
        val prefs = getSharedPreferences(PREFS, MODE_PRIVATE)
        val lang = prefs.getString(KEY_LANG, "fa") ?: "fa"
        // Persian is the first-class language; layout direction follows locale.
        AppCompatDelegate.setApplicationLocales(LocaleListCompat.forLanguageTags(lang))
    }

    companion object {
        const val PREFS = "generals_prefs"
        const val KEY_LANG = "ui_language"
        const val KEY_GAME_URI = "game_data_uri"
        const val KEY_GAME_TYPE = "game_type"        // "zh" | "generals"
        const val KEY_NUMERALS = "persian_numerals"  // true=۱۲۳ false=123
        const val KEY_NICK = "nickname"
        const val KEY_GAME_NAME = "lobby_game_name"
        const val KEY_RENDERER = "renderer"          // auto|gles|vulkan|safe
        const val KEY_TOUCH_SENS = "touch_sens"
        const val KEY_CAMERA_SENS = "camera_sens"
        const val KEY_HUD_SCALE = "hud_scale"
        const val KEY_LEFT_HANDED = "left_handed"
        const val KEY_REDUCED_ANIM = "reduced_anim"
        const val KEY_EDGE_SCROLL = "edge_scroll"
        const val KEY_WIZARD_DONE = "wizard_done"
    }
}
