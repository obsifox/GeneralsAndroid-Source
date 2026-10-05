package com.obsifox.generals.game

import android.content.Intent
import android.os.Bundle
import android.view.Gravity
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.LinearLayout
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat
import com.obsifox.generals.MainActivity
import com.obsifox.generals.R
import com.obsifox.generals.io.GameDataResolver
import com.obsifox.generals.io.GameDataStore
import com.obsifox.generals.io.GameFilesValidator
import com.obsifox.generals.lobby.LobbyActivity
import com.obsifox.generals.nativelib.NativeBridge
import com.obsifox.generals.util.Persian
import com.obsifox.generals.util.Ui

/**
 * GameActivity (v0.1.3) — the screen the user actually lands on after tapping
 * the app icon. Fullscreen, landscape, immersive: boots the native platform
 * layer against a REAL readable game-data path (raw path if possible, else the
 * one-time local copy) and reports honest engine status (§20 — never faked).
 */
class GameActivity : AppCompatActivity() {

    private lateinit var statusLine: TextView
    private lateinit var detailLine: TextView
    private lateinit var buttons: LinearLayout

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // fullscreen immersive
        WindowCompat.setDecorFitsSystemWindows(window, false)
        val controller = WindowInsetsControllerCompat(window, window.decorView)
        controller.hide(WindowInsetsCompat.Type.systemBars())
        controller.systemBarsBehavior =
            WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE

        val root = FrameLayout(this)
        root.setBackgroundColor(getColor(R.color.bg_dark))
        root.keepScreenOn = true

        val content = Ui.linear(this)
        content.setPadding(Ui.dp(content, 28), Ui.dp(content, 26), Ui.dp(content, 28),
            Ui.dp(content, 20))
        content.gravity = Gravity.CENTER_HORIZONTAL
        root.addView(content, FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT,
            Gravity.CENTER))

        val wordmark = Ui.centerTitle(this, getString(R.string.app_name))
        content.addView(wordmark)

        statusLine = Ui.label(this, getString(R.string.game_loading), 16,
            bold = true, color = R.color.accent)
        statusLine.gravity = Gravity.CENTER
        content.addView(statusLine); Ui.margins(statusLine, top = 12)

        detailLine = Ui.label(this, "", 12, color = R.color.text_secondary)
        detailLine.gravity = Gravity.CENTER
        content.addView(detailLine); Ui.margins(detailLine, top = 6)

        buttons = Ui.linear(this)
        content.addView(buttons)

        setContentView(root)
        bootEngine()
    }

    private fun bootEngine() {
        Thread {
            NativeBridge.ensureLoaded()

            // safety re-validation (keeps §14 guarantees)
            val uri = GameDataStore.gameDataUri(this)
            val res = GameFilesValidator.validate(this, uri)
            if (!res.ok) {
                ui {
                    statusLine.text = getString(R.string.game_data_fail)
                    detailLine.text = res.errors.joinToString("\n") { "✕ $it" }
                    addButtons(showLan = false)
                }
                return@Thread
            }

            // real readable path for the native layer (zero-copy or one-time copy)
            val path = GameDataResolver.ensureRealPath(this, uri) { file, count ->
                ui {
                    detailLine.text =
                        getString(R.string.game_copying, Persian.num(this@GameActivity, count),
                            file)
                }
            }

            if (path == null) {
                ui {
                    statusLine.text = getString(R.string.game_data_fail)
                    detailLine.text = getString(R.string.game_data_fail_hint)
                    addButtons(showLan = false)
                }
                return@Thread
            }

            val ok = try {
                NativeBridge.nativeStartEngine(arrayOf(path, filesDir.absolutePath))
            } catch (t: Throwable) { false }
            val version = try { NativeBridge.engineVersion() } catch (t: Throwable) { "?" }

            ui {
                statusLine.text = if (ok) getString(R.string.game_engine_ready)
                    else getString(R.string.game_engine_boot_failed)
                detailLine.text =
                    getString(R.string.game_engine_version, version) + "\n" +
                    getString(R.string.game_engine_note) + "\n" +
                    getString(R.string.game_data_path, path)
                addButtons(showLan = true)
            }
        }.start()
    }

    private fun addButtons(showLan: Boolean) {
        buttons.removeAllViews()
        if (showLan) {
            buttons.addView(Ui.menuButton(this, getString(R.string.game_btn_lan)) {
                startActivity(Intent(this, LobbyActivity::class.java))
            })
        }
        buttons.addView(Ui.menuButton(this, getString(R.string.game_btn_dashboard)) {
            startActivity(Intent(this, MainActivity::class.java))
        })
        buttons.addView(Ui.menuButton(this, getString(R.string.game_btn_exit)) {
            try { NativeBridge.nativeShutdownEngine() } catch (t: Throwable) { }
            finishAffinity()
        })
        // slightly denser in-game buttons (landscape space)
        for (i in 0 until buttons.childCount) {
            val b = buttons.getChildAt(i)
            b.setPadding(Ui.dp(b, 16), Ui.dp(b, 10), Ui.dp(b, 16), Ui.dp(b, 10))
        }
    }

    private fun ui(block: () -> Unit) = runOnUiThread(block)
}
