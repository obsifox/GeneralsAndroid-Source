package com.obsifox.generals.settings

import android.os.Bundle
import android.view.ViewGroup
import android.widget.CheckBox
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.SeekBar
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.os.LocaleListCompat
import com.obsifox.generals.GeneralsApp
import com.obsifox.generals.R
import com.obsifox.generals.util.Persian
import com.obsifox.generals.util.Ui

/**
 * Settings (Master Prompt §22, §23): language, numerals, renderer preference,
 * touch/camera sensitivity, HUD scale, accessibility toggles. Persisted locally.
 */
class SettingsActivity : AppCompatActivity() {

    private val prefs by lazy { Persian.prefs(this) }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.statusBarColor = getColor(R.color.status_bar)

        val root = ScrollView(this)
        val box = Ui.linear(this)
        box.setPadding(Ui.dp(box, 20), Ui.dp(box, 24), Ui.dp(box, 20), Ui.dp(box, 20))
        root.addView(box, ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))
        box.addView(Ui.centerTitle(this, getString(R.string.settings_title)))
        Ui.margins(box.getChildAt(box.childCount - 1), bottom = 16)

        // — زبان —
        box.addView(Ui.sectionLabel(getString(R.string.settings_language)))
        box.addView(Ui.menuButton(this, getString(R.string.settings_lang_fa)) { setLang("fa") })
        box.addView(Ui.menuButton(this, getString(R.string.settings_lang_en)) { setLang("en") })

        // — اعداد —
        box.addView(Ui.sectionLabel(getString(R.string.settings_numerals)))
        box.addView(Ui.menuButton(this, getString(R.string.settings_numerals_persian)) {
            prefs.edit().putBoolean(GeneralsApp.KEY_NUMERALS, true).apply(); saved() })
        box.addView(Ui.menuButton(this, getString(R.string.settings_numerals_latin)) {
            prefs.edit().putBoolean(GeneralsApp.KEY_NUMERALS, false).apply(); saved() })

        // — رندرر —
        box.addView(Ui.sectionLabel(getString(R.string.settings_renderer)))
        for (pair in listOf(
            "auto" to getString(R.string.settings_renderer_auto),
            "gles" to getString(R.string.settings_renderer_gles),
            "vulkan" to getString(R.string.settings_renderer_vulkan),
            "safe" to getString(R.string.settings_renderer_safe))) {
            box.addView(Ui.menuButton(this, pair.second) {
                prefs.edit().putString(GeneralsApp.KEY_RENDERER, pair.first).apply(); saved() })
        }

        // — حساسیت‌ها (بخش ۸: قابل پیکربندی و persist) —
        box.addView(Ui.sectionLabel(getString(R.string.settings_touch)))
        box.addView(slider(GeneralsApp.KEY_TOUCH_SENS, 50))
        box.addView(Ui.sectionLabel(getString(R.string.settings_camera)))
        box.addView(slider(GeneralsApp.KEY_CAMERA_SENS, 50))
        box.addView(Ui.sectionLabel(getString(R.string.settings_hud_scale)))
        box.addView(slider(GeneralsApp.KEY_HUD_SCALE, 70))

        // — دسترس‌پذیری —
        box.addView(Ui.sectionLabel(getString(R.string.settings_left_handed)))
        box.addView(toggle(GeneralsApp.KEY_LEFT_HANDED))
        box.addView(Ui.sectionLabel(getString(R.string.settings_reduce_anim)))
        box.addView(toggle(GeneralsApp.KEY_REDUCED_ANIM))
        box.addView(Ui.sectionLabel(getString(R.string.settings_edge_scroll)))
        box.addView(toggle(GeneralsApp.KEY_EDGE_SCROLL))

        setContentView(root)
    }

    private fun Ui.sectionLabel(text: String): TextView {
        val t = Ui.label(this@SettingsActivity, text, 15, bold = true,
            color = R.color.accent)
        Ui.margins(t, top = 18, bottom = 4)
        return t
    }

    private fun slider(key: String, default: Int): LinearLayout {
        val value = prefs.getInt(key, default)
        val row = Ui.linear(this, vertical = false)
        val label = Ui.label(this, Persian.num(this, value), 15, bold = true)
        val bar = SeekBar(this)
        bar.max = 100
        bar.progress = value
        bar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(sb: SeekBar?, progress: Int, fromUser: Boolean) {
                label.text = Persian.num(this@SettingsActivity, progress)
            }
            override fun onStartTrackingTouch(sb: SeekBar?) {}
            override fun onStopTrackingTouch(sb: SeekBar?) {
                prefs.edit().putInt(key, sb?.progress ?: default).apply()
            }
        })
        row.addView(label, LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT))
        row.addView(bar, LinearLayout.LayoutParams(0,
            ViewGroup.LayoutParams.WRAP_CONTENT, 1f))
        return row
    }

    private fun toggle(key: String): CheckBox {
        val c = CheckBox(this)
        c.isChecked = prefs.getBoolean(key, false)
        c.setTextColor(getColor(R.color.text_primary))
        c.setOnCheckedChangeListener { _, checked ->
            prefs.edit().putBoolean(key, checked).apply()
        }
        return c
    }

    private fun setLang(tag: String) {
        prefs.edit().putString(GeneralsApp.KEY_LANG, tag).apply()
        AppCompatDelegate.setApplicationLocales(LocaleListCompat.forLanguageTags(tag))
        recreate()
    }

    private fun saved() {
        AlertDialog.Builder(this)
            .setMessage(getString(R.string.settings_saved))
            .setPositiveButton(getString(R.string.ok), null)
            .show()
    }
}
