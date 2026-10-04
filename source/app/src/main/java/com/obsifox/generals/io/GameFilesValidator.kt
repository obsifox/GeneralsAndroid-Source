package com.obsifox.generals.io

import android.content.Context
import android.net.Uri
import androidx.documentfile.provider.DocumentFile

/**
 * GameDataValidator (Master Prompt §14):
 * - checks required data via the user-selected SAF folder
 * - NEVER crashes on missing data; produces structured, localizable results
 * - treats imported files as untrusted input (§34): only names/sizes are read
 */
data class ValidationResult(
    val ok: Boolean,
    val bigCount: Int,
    val markerFile: String?,   // generalszh.exe | generals.exe
    val csfFile: String?,
    val errors: List<String>,
    val notes: List<String>
)

object GameFilesValidator {

    private val REQUIRED_BIG_MIN = 1

    fun validate(context: Context, treeUriString: String?): ValidationResult {
        if (treeUriString.isNullOrBlank()) {
            return ValidationResult(false, 0, null, null,
                listOf("validate_no_folder"), emptyList())
        }
        return try {
            val root = DocumentFile.fromTreeUri(context, Uri.parse(treeUriString))
            if (root == null || !root.canRead()) {
                ValidationResult(false, 0, null, null,
                    listOf("validate_folder_read_error", treeUriString), emptyList())
            } else validateRoot(context, root)
        } catch (t: Throwable) {
            // حداکثر محافظت: هیچ ورودی کاربر نباید کرش ایجاد کند
            ValidationResult(false, 0, null, null,
                listOf("validate_folder_read_error", t.message ?: "?"), emptyList())
        }
    }

    private fun validateRoot(context: Context, root: DocumentFile): ValidationResult {
        val errors = mutableListOf<String>()
        val notes = mutableListOf<String>()

        val files = root.listFiles()
        var bigCount = 0
        var marker: String? = null
        var csf: String? = null
        // also scan one level deep (retail layouts put .big under Data/…)
        val subDirs = mutableListOf<DocumentFile>()

        for (f in files) {
            val name = f.name?.lowercase() ?: continue
            if (f.isDirectory) {
                subDirs.add(f)
                continue
            }
            if (name.endsWith(".big")) bigCount++
            if (name == "generalszh.exe") marker = f.name
            if (name == "generals.exe" && marker == null) marker = f.name
            if (name.endsWith(".csf") && csf == null) csf = f.name
        }
        if (bigCount < REQUIRED_BIG_MIN) {
            for (d in subDirs.take(12)) {
                for (f in d.listFiles()) {
                    val name = f.name?.lowercase() ?: continue
                    if (!f.isDirectory) {
                        if (name.endsWith(".big")) bigCount++
                        if (name == "generalszh.exe" && marker == null) marker = f.name
                        if (name == "generals.exe" && marker == null) marker = f.name
                        if (name.endsWith(".csf") && csf == null) csf = f.name
                    }
                }
            }
        }

        notes.add(res(context, "validate_found_big_count", bigCount))
        if (marker != null) notes.add(res(context, "validate_found_marker", marker))
        if (csf != null) notes.add(res(context, "validate_found_csf", csf))

        if (bigCount < REQUIRED_BIG_MIN) errors.add(res(context, "validate_missing_big"))
        if (marker == null) errors.add(res(context, "validate_missing_exe"))
        if (csf == null) errors.add(res(context, "validate_missing_csf"))

        val ok = bigCount >= REQUIRED_BIG_MIN // مهر نسخه/CSF هشدار هستند نه رد مطلق
        return ValidationResult(ok, bigCount, marker, csf, errors, notes)
    }

    private fun res(context: Context, key: String, vararg args: Any): String =
        try {
            val id = context.resources.getIdentifier(key, "string", context.packageName)
            if (id != 0) context.getString(id, *args) else key
        } catch (t: Throwable) { key }
}
