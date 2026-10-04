package com.obsifox.generals.util

import android.content.Context
import android.content.SharedPreferences

/**
 * Persian numeral conversion (Master Prompt §9: user-selectable ۱۲۳/123).
 * NO naive character reversal anywhere — digits only, text itself is handled
 * by the platform's Unicode bidi engine.
 */
object Persian {

    private val faDigits = charArrayOf('۰', '۱', '۲', '۳', '۴', '۵', '۶', '۷', '۸', '۹')

    fun toPersianDigits(input: String): String {
        val sb = StringBuilder(input.length)
        for (c in input) {
            if (c in '0'..'9') sb.append(faDigits[c - '0']) else sb.append(c)
        }
        return sb.toString()
    }

    /** Formats an integer according to the user's numeral preference. */
    fun num(context: Context, value: Int): String {
        val s = value.toString()
        return if (prefs(context).getBoolean("persian_numerals", true)) toPersianDigits(s) else s
    }

    fun prefs(context: Context): SharedPreferences =
        context.getSharedPreferences("generals_prefs", Context.MODE_PRIVATE)
}
