package com.obsifox.generals.wizard

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.view.Gravity
import android.view.ViewGroup
import android.widget.Button
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.os.LocaleListCompat
import com.obsifox.generals.GeneralsApp
import com.obsifox.generals.R
import com.obsifox.generals.io.GameDataStore
import com.obsifox.generals.io.GameFilesValidator
import com.obsifox.generals.io.ValidationResult
import com.obsifox.generals.util.Persian
import com.obsifox.generals.util.Ui

/**
 * First-run wizard (Master Prompt §13, §32):
 * WELCOME → LANGUAGE → SELECT GAME → IMPORT (SAF) → VALIDATE → READY
 * Missing data NEVER crashes — errors are localized and structured (§14).
 */
class FirstRunWizardActivity : AppCompatActivity() {

    private var step = 0
    private var gameType = "zh"
    private var pickedUri: Uri? = null
    private var lastResult: ValidationResult? = null

    private lateinit var body: LinearLayout
    private lateinit var btnNext: Button
    private lateinit var btnBack: Button
    private lateinit var btnFinish: Button

    private val openFolder = registerForActivityResult(
        androidx.activity.result.contract.ActivityResultContracts.OpenDocumentTree()
    ) { uri: Uri? ->
        if (uri != null) {
            try {
                contentResolver.takePersistableUriPermission(
                    uri,
                    Intent.FLAG_GRANT_READ_URI_PERMISSION or
                        Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                )
            } catch (t: SecurityException) { /* read-only grant is still usable */ }
            pickedUri = uri
            render()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.statusBarColor = getColor(R.color.status_bar)

        val outer = ScrollView(this)
        body = Ui.linear(this)
        body.setPadding(Ui.dp(body, 20), Ui.dp(body, 28), Ui.dp(body, 20), Ui.dp(body, 20))
        outer.addView(body, ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))
        setContentView(outer)
        render()
    }

    private fun render() {
        body.removeAllViews()
        val header = Ui.centerTitle(this, getString(R.string.wizard_title) +
            " (${Persian.num(this, step + 1)}/5)")
        body.addView(header); Ui.margins(header, bottom = 18)

        when (step) {
            0 -> renderWelcome()
            1 -> renderLanguage()
            2 -> renderSelectGame()
            3 -> renderImport()
            4 -> renderValidate()
        }
        body.addView(buildNavButtons())
    }

    private fun renderWelcome() {
        body.addView(Ui.label(this, getString(R.string.wizard_welcome), 20, bold = true,
            color = R.color.accent))
        val t = Ui.label(this, getString(R.string.wizard_welcome_body), 15)
        t.setLineSpacing(0f, 1.25f)
        body.addView(t); Ui.margins(t, top = 12)
        body.addView(Ui.label(this, getString(R.string.lobby_hotspot_hint), 12,
            color = R.color.text_secondary))
    }

    private fun renderLanguage() {
        body.addView(Ui.label(this, getString(R.string.wizard_step_language), 17, bold = true))
        body.addView(Ui.menuButton(this, getString(R.string.settings_lang_fa)) {
            setLang("fa") })
        body.addView(Ui.menuButton(this, getString(R.string.settings_lang_en)) {
            setLang("en") })
    }

    private fun setLang(tag: String) {
        Persian.prefs(this).edit().putString(GeneralsApp.KEY_LANG, tag).apply()
        AppCompatDelegate.setApplicationLocales(LocaleListCompat.forLanguageTags(tag))
        recreate()
    }

    private fun renderSelectGame() {
        body.addView(Ui.label(this, getString(R.string.wizard_step_game), 17, bold = true))
        body.addView(Ui.menuButton(this, getString(R.string.wizard_game_zh)) {
            gameType = "zh"; step = 3; render() })
        body.addView(Ui.menuButton(this, getString(R.string.wizard_game_generals)) {
            gameType = "generals"; step = 3; render() })
    }

    private fun renderImport() {
        body.addView(Ui.label(this, getString(R.string.wizard_step_folder), 17, bold = true))
        val hint = Ui.label(this, getString(R.string.wizard_folder_hint), 14,
            color = R.color.text_secondary)
        hint.setLineSpacing(0f, 1.2f)
        body.addView(hint); Ui.margins(hint, top = 10, bottom = 12)
        body.addView(Ui.menuButton(this, getString(R.string.wizard_pick_folder)) {
            openFolder.launch(null) })
        pickedUri?.let {
            body.addView(Ui.label(this, "▸ ${it.lastPathSegment ?: it}", 13,
                color = R.color.ok_green))
        }
    }

    private fun renderValidate() {
        body.addView(Ui.label(this, getString(R.string.wizard_step_validate), 17, bold = true))
        val res = lastResult ?: GameFilesValidator.validate(this, pickedUri?.toString()
            ?: GameDataStore.gameDataUri(this)).also { lastResult = it }

        val card = Ui.card(this)
        if (res.ok) {
            card.addView(Ui.label(this, getString(R.string.validate_ok_header), 15,
                bold = true, color = R.color.ok_green))
        } else {
            card.addView(Ui.label(this, getString(R.string.validate_problem_header), 15,
                bold = true, color = R.color.warn_red))
        }
        res.notes.forEach { n ->
            card.addView(Ui.label(this, "✓ $n", 13, color = R.color.text_primary))
        }
        res.errors.forEach { e ->
            card.addView(Ui.label(this, "✕ $e", 13, color = R.color.warn_red))
        }
        if (res.ok && pickedUri != null) {
            GameDataStore.saveGameData(this, pickedUri.toString(), gameType)
        }
        body.addView(card); Ui.margins(card, top = 12)
        body.addView(Ui.menuButton(this, getString(R.string.data_status_action)) {
            lastResult = null; openFolder.launch(null) })
    }

    private fun buildNavButtons(): LinearLayout {
        val row = Ui.linear(this, vertical = false)
        btnBack = Ui.menuButton(this, getString(R.string.wizard_btn_back)) {
            if (step > 0) { step--; if (step == 3 && pickedUri == null && lastResult != null) lastResult = null; render() }
        }
        btnNext = Ui.menuButton(this, getString(R.string.wizard_btn_next)) {
            if (step in listOf(0, 1)) { step++; render() }
            else if (step == 3 && pickedUri != null) { step++; lastResult = null; render() }
        }
        btnFinish = Ui.menuButton(this, getString(R.string.wizard_btn_finish)) {
            finish()
        }
        btnNext.gravity = Gravity.CENTER
        row.addView(btnBack)
        Ui.margins(btnBack, right = 6)
        row.addView(btnNext)
        Ui.margins(btnNext, left = 6)
        // Finish only appears when validation passed
        btnFinish.visibility = if (lastResult?.ok == true) View.VISIBLE else View.GONE
        val wrap = Ui.linear(this)
        wrap.addView(row)
        wrap.addView(btnFinish)
        return wrap
    }
}
