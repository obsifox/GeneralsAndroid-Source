package com.obsifox.generals.io

import android.content.Context
import android.net.Uri
import androidx.documentfile.provider.DocumentFile
import java.io.File
import java.io.FileOutputStream

/**
 * GameDataResolver — makes «کلیک → اجرای مستقیم بازی» possible (v0.1.3):
 *
 * The native engine needs a REAL filesystem path (stat/opendir), not a SAF
 * tree URI. Two strategies, tried in order:
 *   1) zero-copy: convert a primary-storage tree URI to a raw path that the
 *      process can actually open;
 *   2) one-time local copy into filesDir/game_data — after this single copy
 *      every future launch is instant, offline, and needs NO permissions,
 *      NO picker, NO wizard again.
 * Never crashes on bad input (§14): errors are returned, not thrown.
 */
object GameDataResolver {

    /** User-visible progress callback (file name + copied count). */
    fun interface Progress {
        fun onProgress(fileName: String, filesDone: Int)
    }

    /**
     * Returns a real readable directory path for the given SAF tree URI,
     * or null when neither strategy works. [onProgress] fires only during
     * the one-time copy.
     */
    fun ensureRealPath(
        context: Context,
        treeUriString: String?,
        onProgress: Progress? = null
    ): String? {
        if (treeUriString.isNullOrBlank()) return null
        return try {
            val uri = Uri.parse(treeUriString)
            val raw = rawPathFromTreeUri(uri)
            if (isReadableDir(raw)) return raw

            val root = DocumentFile.fromTreeUri(context, uri) ?: return null
            if (!root.canRead()) return null

            val dest = File(context.filesDir, "game_data")
            return if (copyTree(context, root, dest, onProgress)) dest.absolutePath else null
        } catch (t: Throwable) {
            null // §14: never crash on user data
        }
    }

    /**
     * content://com.android.externalstorage.documents/tree/primary%3AGames%2FGenerals
     *   → /storage/emulated/0/Games/Generals
     */
    fun rawPathFromTreeUri(uri: Uri): String? {
        if (uri.authority != "com.android.externalstorage.documents") return null
        val tree = uri.lastPathSegment ?: return null
        val colon = tree.indexOf(':')
        if (colon <= 0) return null
        val volume = tree.substring(0, colon)
        val rest = tree.substring(colon + 1)
        val base = if (volume == "primary") "/storage/emulated/0" else "/storage/$volume"
        return if (rest.isBlank()) base else "$base/$rest"
    }

    private fun isReadableDir(path: String?): Boolean {
        if (path == null) return false
        return try {
            val f = File(path)
            f.isDirectory && f.canRead() && (f.list()?.isNotEmpty() == true)
        } catch (t: Throwable) {
            false
        }
    }

    /**
     * Recursive copy with resume (same-length files are skipped, so an
     * interrupted first import continues where it stopped). Per-file errors
     * are tolerated — only a directory-level failure aborts.
     */
    private fun copyTree(
        context: Context,
        src: DocumentFile,
        dst: File,
        onProgress: Progress?
    ): Boolean {
        return try {
            if (!dst.exists() && !dst.mkdirs() && !dst.isDirectory) return false
            var done = 0
            for (child in src.listFiles()) {
                val name = child.name ?: continue
                val target = File(dst, name)
                if (child.isDirectory) {
                    if (!copyTree(context, child, target, onProgress)) return false
                } else {
                    if (!target.exists() || target.length() != child.length()) {
                        if (!copyViaResolver(context, child, target)) {
                            // per-file tolerance: skip broken file, keep going
                            continue
                        }
                    }
                    done++
                    onProgress?.onProgress(name, done)
                }
            }
            true
        } catch (t: Throwable) {
            false
        }
    }

    /** Streaming copy through ContentResolver (SAF has no raw-path guarantee). */
    private fun copyViaResolver(context: Context, child: DocumentFile, target: File): Boolean {
        return try {
            context.contentResolver.openInputStream(child.uri)?.use { inS ->
                FileOutputStream(target).use { outS ->
                    val buf = ByteArray(1024 * 1024)
                    while (true) {
                        val n = inS.read(buf)
                        if (n < 0) break
                        outS.write(buf, 0, n)
                    }
                    outS.flush()
                }
            } ?: return false
            target.length() == child.length()
        } catch (t: Throwable) {
            false
        }
    }
}
