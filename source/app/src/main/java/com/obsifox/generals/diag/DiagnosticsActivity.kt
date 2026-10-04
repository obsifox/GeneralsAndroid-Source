package com.obsifox.generals.diag

import android.app.ActivityManager
import android.content.Context
import android.opengl.GLES10
import android.os.Build
import android.os.Bundle
import android.view.ViewGroup
import android.widget.LinearLayout
import android.widget.ScrollView
import androidx.appcompat.app.AppCompatActivity
import com.obsifox.generals.R
import com.obsifox.generals.nativelib.NativeBridge
import com.obsifox.generals.util.Persian
import com.obsifox.generals.util.Ui
import java.io.File
import javax.microedition.khronos.egl.EGL10
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.egl.EGLContext
import javax.microedition.khronos.egl.EGLDisplay

/**
 * Diagnostics + device capability profile (Master Prompt §24, §25).
 * Detects GPU/RAM/ABI/refresh-rate and maps to HIGH/MEDIUM/LOW/SAFE.
 */
class DiagnosticsActivity : AppCompatActivity() {

    private val rows = mutableListOf<Pair<String, String>>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.statusBarColor = getColor(R.color.status_bar)

        val root = ScrollView(this)
        val box = Ui.linear(this)
        box.setPadding(Ui.dp(box, 20), Ui.dp(box, 24), Ui.dp(box, 20), Ui.dp(box, 20))
        root.addView(box, ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))

        box.addView(Ui.centerTitle(this, getString(R.string.diag_title)))
        Ui.margins(box.getChildAt(box.childCount - 1), bottom = 16)

        val gpu = queryGpu()
        val ramMb = queryRamMb()
        val profile = compatProfile(ramMb, gpu)

        rows.add(getString(R.string.diag_device) to "${Build.MANUFACTURER} ${Build.MODEL}")
        rows.add(getString(R.string.diag_android) to "API ${Persian.num(this, Build.VERSION.SDK_INT)}")
        rows.add(getString(R.string.diag_abi) to Build.SUPPORTED_ABIS.joinToString(", "))
        rows.add(getString(R.string.diag_ram) to "~${Persian.num(this, ramMb)} MB")
        rows.add(getString(R.string.diag_gpu) to gpu)
        rows.add(getString(R.string.diag_compat_profile) to profile)

        val card = Ui.card(this)
        for ((k, v) in rows) {
            val line = Ui.linear(this, vertical = false)
            val kt = Ui.label(this, "$k:", 13, bold = true, color = R.color.text_secondary)
            val vt = Ui.label(this, v, 13)
            line.addView(kt, LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 0.42f))
            line.addView(vt, LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 0.58f))
            card.addView(line); Ui.margins(line, top = 6)
        }
        box.addView(card)
        Ui.margins(card, top = 6, bottom = 14)

        // موتور native
        val engineCard = Ui.card(this)
        engineCard.addView(Ui.label(this, getString(R.string.diag_engine), 15, bold = true,
            color = R.color.accent))
        val loaded = NativeBridge.ensureLoaded()
        val version = if (loaded) runCatching { NativeBridge.engineVersion() }
            .getOrDefault("(init error)") else "(not loaded)"
        engineCard.addView(Ui.label(this,
            getString(R.string.diag_engine_version) + " " + version, 13))
        if (loaded) {
            runCatching { NativeBridge.nativeDeviceProfile() }.getOrNull()?.let {
                engineCard.addView(Ui.label(this, "▸ $it", 12, color = R.color.text_secondary))
            }
        }
        box.addView(engineCard)

        // لاگ‌های قابل‌خواندن (بخش ۲۴: هرگز خطاها را قورت نده)
        box.addView(Ui.section("Crash log", this))
        val logCard = Ui.card(this)
        val logFile = File(filesDir, "diagnostics.log")
        val logText = if (logFile.exists()) logFile.readText().takeLast(1500) else "—"
        logCard.addView(Ui.label(this, logText, 11, color = R.color.text_secondary))
        box.addView(logCard)

        setContentView(root)
    }

    private fun Ui.section(text: String, ctx: Context) =
        Ui.label(ctx, text, 15, bold = true, color = R.color.accent).also {
            Ui.margins(it, top = 18, bottom = 4)
        }

    private fun queryGpu(): String {
        return try {
            var renderer = "?"
            val egl = EGLContext.getEGL() as EGL10
            val display: EGLDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY)
            if (display != EGL10.EGL_NO_DISPLAY) {
                val ver = IntArray(2)
                if (egl.eglInitialize(display, ver)) {
                    val cfgAttribs = intArrayOf(
                        EGL10.EGL_SURFACE_TYPE, EGL10.EGL_PBUFFER_BIT,
                        EGL10.EGL_RED_SIZE, 5, EGL10.EGL_GREEN_SIZE, 6,
                        EGL10.EGL_BLUE_SIZE, 5, EGL10.EGL_NONE)
                    val configs = arrayOfNulls<EGLConfig>(1)
                    val num = IntArray(1)
                    if (egl.eglChooseConfig(display, cfgAttribs, configs, 1, num) && num[0] > 0) {
                        val surface = egl.eglCreatePbufferSurface(display, configs[0],
                            intArrayOf(EGL10.EGL_WIDTH, 1, EGL10.EGL_HEIGHT, 1, EGL10.EGL_NONE))
                        if (surface != null && surface != EGL10.EGL_NO_SURFACE) {
                            val ctx = egl.eglCreateContext(display, configs[0], EGL10.EGL_NO_CONTEXT, null)
                            if (ctx != null && ctx != EGL10.EGL_NO_CONTEXT) {
                                if (egl.eglMakeCurrent(display, surface, surface, ctx)) {
                                    renderer = GLES10.glGetString(GLES10.GL_RENDERER) ?: "?"
                                    egl.eglMakeCurrent(display, EGL10.EGL_NO_SURFACE,
                                        EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT)
                                }
                                egl.eglDestroyContext(display, ctx)
                            }
                            egl.eglDestroySurface(display, surface)
                        }
                    }
                    egl.eglTerminate(display)
                }
            }
            renderer
        } catch (t: Throwable) {
            "unknown (${t.javaClass.simpleName})"
        }
    }

    private fun queryRamMb(): Int {
        val am = getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val mem = ActivityManager.MemoryInfo()
        am.getMemoryInfo(mem)
        return (mem.totalMem / (1024L * 1024L)).toInt()
    }

    /** بخش ۲۵: پروفایل سازگاری HIGH/MEDIUM/LOW/SAFE */
    private fun compatProfile(ramMb: Int, gpu: String): String {
        val g = gpu.lowercase()
        val modernGpu = listOf("adreno 6", "adreno 7", "adreno 8", "mali-g7", "mali-g6",
            "xclipse", "immortalis").any { g.contains(it) }
        return when {
            ramMb >= 6000 && modernGpu -> getString(R.string.diag_compat_high)
            ramMb >= 3000 -> getString(R.string.diag_compat_medium)
            ramMb >= 2000 -> getString(R.string.diag_compat_low)
            else -> getString(R.string.diag_compat_safe)
        }
    }
}
