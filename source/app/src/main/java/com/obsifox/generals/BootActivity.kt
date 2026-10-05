package com.obsifox.generals

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.os.LocaleListCompat
import com.obsifox.generals.io.GameDataResolver
import com.obsifox.generals.io.GameDataStore
import com.obsifox.generals.io.GameFilesValidator
import com.obsifox.generals.io.ValidationResult
import com.obsifox.generals.util.Persian
import com.obsifox.generals.util.Ui

/**
 * BootActivity (v0.1.3) — the ONLY launcher entry.
 * «کلیک روی بازی → خودِ بازی اجرا می‌شود»:
 *
 *   tap icon → (files ready?) → GameActivity directly.
 *   Only the very first run shows ONE screen asking for the game folder
 *   once — no multi-step wizard, no menus in between. After that every
 *   launch goes straight into the game.
 */
class BootActivity : AppCompatActivity() {

    private enum class Mode { LOADING, NEED_FILES, PREPARING, ERROR }

    private lateinit var body: LinearLayout
    private var progressText: TextView? = null
    private var pendingUri: Uri? = null

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
            pendingUri = uri
            importAndLaunch(uri.toString())
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.statusBarColor = getColor(R.color.status_bar)

        body = Ui.linear(this)
        body.setPadding(Ui.dp(body, 24), Ui.dp(body, 40), Ui.dp(body, 24), Ui.dp(body, 24))
        body.setBackgroundColor(getColor(R.color.bg_dark))
        setContentView(body)

        render(Mode.LOADING, null)
        Thread {
            val configured = GameDataStore.isConfigured(this)
            val res = if (configured)
                GameFilesValidator.validate(this, GameDataStore.gameDataUri(this)) else null
            runOnUiThread {
                if (res != null && res.ok) enterGame() else render(Mode.NEED_FILES, res)
            }
        }.start()
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        // Re-tap on the icon while we are alive → straight into the game.
        if (GameDataStore.isConfigured(this)) enterGame()
    }

    // ————————————————————————————————————— screens —————————————————————————

    private fun render(mode: Mode, res: ValidationResult?) {
        body.removeAllViews()

        when (mode) {
            Mode.LOADING -> {
                body.gravity = Gravity.CENTER
                body.addView(Ui.centerTitle(this, getString(R.string.app_name)))
                val p = Ui.label(this, getString(R.string.boot_checking), 15,
                    color = R.color.text_secondary)
                p.gravity = Gravity.CENTER
                body.addView(p); Ui.margins(p, top = 14)
            }

            Mode.PREPARING -> {
                body.gravity = Gravity.CENTER
                body.addView(Ui.centerTitle(this, getString(R.string.app_name)))
                val p = Ui.label(this, getString(R.string.boot_preparing), 15,
                    color = R.color.accent)
                p.gravity = Gravity.CENTER
                body.addView(p); Ui.margins(p, top = 14)
                progressText = Ui.label(this, "", 12, color = R.color.text_secondary)
                progressText?.gravity = Gravity.CENTER
                body.addView(progressText); Ui.margins(progressText, top = 6)
                val note = Ui.label(this, getString(R.string.boot_copy_note), 12,
                    color = R.color.text_secondary)
                note.gravity = Gravity.CENTER
                body.addView(note); Ui.margins(note, top = 8)
            }

            Mode.NEED_FILES, Mode.ERROR -> {
                body.gravity = Gravity.CENTER_HORIZONTAL
                val wordmark = Ui.centerTitle(this, getString(R.string.app_name))
                body.addView(wordmark)
                val tag = Ui.label(this, getString(R.string.boot_tagline), 13,
                    color = R.color.text_secondary)
                tag.gravity = Gravity.CENTER
                body.addView(tag); Ui.margins(tag, top = 4, bottom = 18)

                // one single card — the entire "setup" experience
                val card = Ui.card(this)
                val title = Ui.label(this,
                    if (mode == Mode.ERROR) getString(R.string.boot_setup_failed)
                    else getString(R.string.boot_need_files_title), 16, bold = true,
                    color = R.color.accent)
                card.addView(title); Ui.margins(title, bottom = 8)

                val text = Ui.label(this, getString(R.string.boot_need_files_body), 14)
                text.setLineSpacing(0f, 1.25f)
                card.addView(text)

                if (res != null && res.errors.isNotEmpty()) {
                    card.addView(Ui.label(this,
                        getString(R.string.validate_problem_header), 13,
                        bold = true, color = R.color.warn_red))
                    res.errors.forEach { e ->
                        card.addView(Ui.label(this, "✕ $e", 12, color = R.color.warn_red))
                    }
                }
                body.addView(card); Ui.margins(card, top = 4, bottom = 14)

                // THE one button → picker → auto game
                body.addView(Ui.menuButton(this, getString(R.string.boot_pick_btn)) {
                    openFolder.launch(null)
                })

                // language quick toggle (no wizard step needed anymore)
                val langRow = Ui.linear(this, vertical = false)
                val fa = Ui.menuButton(this, getString(R.string.settings_lang_fa)) {
                    setLang("fa")
                }
                val en = Ui.menuButton(this, getString(R.string.settings_lang_en)) {
                    setLang("en")
                }
                langRow.addView(fa); Ui.margins(fa, left = 4, right = 4)
                langRow.addView(en); Ui.margins(en, left = 4, right = 4)
                body.addView(langRow)

                // advanced users can still reach the old dashboard
                val adv = Ui.label(this, getString(R.string.boot_advanced_link), 13,
                    color = R.color.text_secondary)
                adv.gravity = Gravity.CENTER
                adv.setPadding(0, Ui.dp(adv, 14), 0, 0)
                adv.setOnClickListener {
                    startActivity(Intent(this, MainActivity::class.java))
                }
                body.addView(adv)
            }
        }
    }

    // ——————————————————————————————————— actions ———————————————————————————

    private fun importAndLaunch(treeUri: String) {
        render(Mode.PREPARING, null)
        Thread {
            val res = GameFilesValidator.validate(this, treeUri)
            if (!res.ok) {
                runOnUiThread { render(Mode.NEED_FILES, res) }
                return@Thread
            }
            // auto-detect game type from the marker file — no extra question
            val type = if (res.markerFile?.lowercase() == "generals.exe") "generals" else "zh"
            GameDataStore.saveGameData(this, treeUri, type)

            val path = GameDataResolver.ensureRealPath(this, treeUri) { file, count ->
                runOnUiThread {
                    // live progress while the one-time copy runs
                    progressText?.text = Persian.num(this@BootActivity, count) +
                        " — " + file
                }
            }
            runOnUiThread {
                if (path != null) enterGame() else render(Mode.NEED_FILES, res)
            }
        }.start()
    }

    private fun enterGame() {
        startActivity(
            Intent(this, GameActivity::class.java)
                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP)
        )
        finish()
    }

    private fun setLang(tag: String) {
        Persian.prefs(this).edit().putString(GeneralsApp.KEY_LANG, tag).apply()
        AppCompatDelegate.setApplicationLocales(LocaleListCompat.forLanguageTags(tag))
        recreate()
    }
}
