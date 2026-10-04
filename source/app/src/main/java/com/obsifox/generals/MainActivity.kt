package com.obsifox.generals

import android.content.Intent
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.obsifox.generals.io.GameDataStore
import com.obsifox.generals.lobby.LobbyActivity
import com.obsifox.generals.diag.DiagnosticsActivity
import com.obsifox.generals.nativelib.NativeBridge
import com.obsifox.generals.settings.SettingsActivity
import com.obsifox.generals.util.Persian
import com.obsifox.generals.util.Ui
import com.obsifox.generals.wizard.FirstRunWizardActivity

/**
 * Persian-first dashboard (Master Prompt §31 launcher layout).
 */
class MainActivity : AppCompatActivity() {

    private lateinit var statusText: TextView
    private lateinit var statusPath: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.statusBarColor = getColor(R.color.status_bar)

        val root = Ui.linear(this)
        root.setPadding(Ui.dp(root, 20), Ui.dp(root, 24), Ui.dp(root, 20), Ui.dp(root, 20))
        root.setBackgroundColor(getColor(R.color.bg_dark))

        val title = Ui.centerTitle(this, getString(R.string.app_name))
        root.addView(title)

        val sub = Ui.label(this, getString(R.string.app_subtitle), 13,
            color = R.color.text_secondary)
        sub.gravity = Gravity.CENTER
        root.addView(sub)
        Ui.margins(sub, top = 4, bottom = 18)

        // —— وضعیت فایل‌های بازی (بخش ۳۱: نمایش وضعیت داده در لانچر) ——
        val card = Ui.card(this)
        val statusHeader = Ui.label(this, getString(R.string.data_status_title), 14,
            bold = true, color = R.color.accent)
        statusText = Ui.label(this, "", 15, bold = true)
        statusPath = Ui.label(this, "", 12, color = R.color.text_secondary)
        val fixBtn = Ui.menuButton(this, getString(R.string.data_status_action)) { openWizard() }
        card.addView(statusHeader)
        card.addView(statusText); Ui.margins(statusText, top = 8)
        card.addView(statusPath); Ui.margins(statusPath, top = 4)
        root.addView(card)
        Ui.margins(card, top = 4, bottom = 12)

        // —— منوی اصلی ——
        root.addView(Ui.menuButton(this, getString(R.string.menu_campaign)) {
            toastComingSoon(13) })
        root.addView(Ui.menuButton(this, getString(R.string.menu_skirmish)) {
            toastComingSoon(14) })
        root.addView(Ui.menuButton(this, getString(R.string.menu_challenge)) {
            toastComingSoon(15) })
        root.addView(Ui.menuButton(this, getString(R.string.menu_multiplayer)) {
            startActivity(Intent(this, LobbyActivity::class.java)) })

        // —— منوی ثانویه ——
        root.addView(Ui.menuButton(this, getString(R.string.menu_game_files)) { openWizard() })
        root.addView(Ui.menuButton(this, getString(R.string.menu_settings)) {
            startActivity(Intent(this, SettingsActivity::class.java)) })
        root.addView(Ui.menuButton(this, getString(R.string.menu_diagnostics)) {
            startActivity(Intent(this, DiagnosticsActivity::class.java)) })
        root.addView(Ui.menuButton(this, getString(R.string.menu_about)) { showAbout() })

        val scroll = ScrollView(this)
        scroll.addView(root,
            ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT))
        setContentView(scroll)
    }

    override fun onResume() {
        super.onResume()
        refreshDataStatus()
        NativeBridge.ensureLoaded()
    }

    private fun refreshDataStatus() {
        val configured = GameDataStore.isConfigured(this)
        if (!configured) {
            statusText.text = getString(R.string.data_status_missing)
            statusText.setTextColor(getColor(R.color.warn_red))
            statusPath.text = getString(R.string.data_path_label) + " " + getString(R.string.data_none)
        } else {
            statusText.text = getString(R.string.data_status_ok)
            statusText.setTextColor(getColor(R.color.ok_green))
            val uri = GameDataStore.gameDataUri(this) ?: ""
            statusPath.text = getString(R.string.data_path_label) + " " +
                uri.takeLast(48).substringAfterLast('%')
        }
    }

    private fun openWizard() {
        startActivity(Intent(this, FirstRunWizardActivity::class.java))
    }

    private fun toastComingSoon(phase: Int) {
        android.widget.Toast.makeText(
            this, getString(R.string.coming_soon, Persian.num(this, phase)),
            android.widget.Toast.LENGTH_SHORT).show()
    }

    private fun showAbout() {
        val msg = getString(R.string.about_body) + "\n\n" +
            getString(R.string.about_version, packageManager
                .getPackageInfo(packageName, 0).versionName)
        androidx.appcompat.app.AlertDialog.Builder(this)
            .setTitle(getString(R.string.about_title))
            .setMessage(msg)
            .setPositiveButton(getString(R.string.ok), null)
            .show()
    }
}
