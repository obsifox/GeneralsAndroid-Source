package com.obsifox.generals.nativelib

/**
 * Bridge to libgenerals.so (ARM64). In this phase the native library provides
 * the Android platform bootstrap; the upstream engine link lands in phases 3–5
 * (see docs/ARCHITECTURE_DECISION.md). All symbols are implemented in
 * src/main/cpp/generals_jni.cpp.
 */
object NativeBridge {

    @Volatile
    private var loaded = false

    @Synchronized
    fun ensureLoaded(): Boolean = try {
        if (!loaded) {
            System.loadLibrary("generals")
            loaded = true
        }
        true
    } catch (t: UnsatisfiedLinkError) {
        false
    }

    external fun engineVersion(): String

    /**
     * Prepares the native platform layer: validates data dir, sets up state.
     * Returns false (never throws) if unusable.
     * [paths]: [0]=game data dir, [1]=app internal files dir
     */
    external fun nativeStartEngine(paths: Array<String>): Boolean

    external fun nativeShutdownEngine()

    external fun nativeDeviceProfile(): String
}
