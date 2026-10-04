package com.obsifox.generals.util

import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.TextView
import androidx.core.content.ContextCompat

/**
 * Small programmatic-UI helpers. UI is built in code (not XML) on purpose:
 * every property is compile-checked, which matters while the project cannot
 * be visually QA'd on real devices yet (Master Prompt §35 — no temporary hacks).
 */
object Ui {

    // Vazirmatn (OFL) — proper Persian typography (Master Prompt §10).
    // Shaping/bidi is handled by the Android platform text engine (§9: no
    // naive character reversal anywhere).
    private var vazirRegular: android.graphics.Typeface? = null
    private var vazirBold: android.graphics.Typeface? = null
    private var fontInit = false

    private fun ensureFonts(context: android.content.Context) {
        if (fontInit) return
        fontInit = true
        try {
            vazirRegular = androidx.core.content.res.ResourcesCompat.getFont(
                context, com.obsifox.generals.R.font.vazirmatn_regular)
            vazirBold = androidx.core.content.res.ResourcesCompat.getFont(
                context, com.obsifox.generals.R.font.vazirmatn_bold)
        } catch (t: Throwable) {
            // fallback: system default still renders Persian correctly
        }
    }

    fun dp(c: android.content.Context, value: Int): Int =
        (value * c.resources.displayMetrics.density + 0.5f).toInt()

    fun dp(v: View, value: Int): Int = dp(v.context, value)

    fun linear(c: android.content.Context, vertical: Boolean = true): LinearLayout {
        val l = LinearLayout(c)
        l.orientation = if (vertical) LinearLayout.VERTICAL else LinearLayout.HORIZONTAL
        return l
    }

    fun label(context: android.content.Context, text: String, sizeSp: Int = 16,
              bold: Boolean = false, color: Int = android.R.color.white): TextView {
        ensureFonts(context)
        val t = TextView(context)
        t.text = text
        t.textSize = sizeSp.toFloat()
        val face = if (bold) vazirBold else vazirRegular
        if (face != null) t.typeface = face else
            t.setTypeface(Typeface.DEFAULT, if (bold) Typeface.BOLD else Typeface.NORMAL)
        t.setTextColor(ContextCompat.getColor(context, color))
        return t
    }

    fun menuButton(context: android.content.Context, text: String, onClick: (View) -> Unit): Button {
        ensureFonts(context)
        val b = Button(context)
        b.text = text
        b.isAllCaps = false
        b.textSize = 17f
        vazirBold?.let { b.typeface = it }
        b.setTextColor(ContextCompat.getColor(context, com.obsifox.generals.R.color.text_primary))
        b.background = ContextCompat.getDrawable(context, com.obsifox.generals.R.drawable.bg_card_accent)
        b.setPadding(dp(b, 18), dp(b, 14), dp(b, 18), dp(b, 14))
        b.setOnClickListener(onClick)
        val p = LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT)
        p.topMargin = dp(b, 10)
        b.layoutParams = p
        return b
    }

    fun card(context: android.content.Context): LinearLayout {
        val c = linear(context)
        c.background = ContextCompat.getDrawable(context, com.obsifox.generals.R.drawable.bg_card)
        c.setPadding(dp(c, 16), dp(c, 14), dp(c, 16), dp(c, 14))
        return c
    }

    fun input(context: android.content.Context, hint: String): EditText {
        val e = EditText(context)
        e.hint = hint
        e.setTextColor(ContextCompat.getColor(context, com.obsifox.generals.R.color.text_primary))
        e.setHintTextColor(ContextCompat.getColor(context, com.obsifox.generals.R.color.text_secondary))
        e.background = ContextCompat.getDrawable(context, com.obsifox.generals.R.drawable.bg_card)
        e.setPadding(dp(e, 14), dp(e, 10), dp(e, 14), dp(e, 10))
        return e
    }

    fun margins(view: View, left: Int = 0, top: Int = 0, right: Int = 0, bottom: Int = 0) {
        val p = view.layoutParams as? LinearLayout.LayoutParams
            ?: LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT)
        p.leftMargin = dp(view, left); p.topMargin = dp(view, top)
        p.rightMargin = dp(view, right); p.bottomMargin = dp(view, bottom)
        view.layoutParams = p
    }

    fun centerTitle(context: android.content.Context, text: String): TextView {
        val t = label(context, text, 22, true, com.obsifox.generals.R.color.accent)
        t.gravity = Gravity.CENTER
        return t
    }
}
