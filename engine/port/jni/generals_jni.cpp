// engine/port/jni/generals_jni.cpp — JNI bridge for the ENGINE-LINKED build.
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
//
// Compiled ONLY when the upstream engine is linked (engine/port/CMakeLists.txt,
// ANDROID branch). It defines the exact same JNI surface as the legacy
// standalone shell (source/app/src/main/cpp/generals_jni.cpp) so the Kotlin
// side (com.obsifox.generals.nativelib.NativeBridge) is identical in both
// build modes — but here nativeStartEngine boots the REAL engine
// (androidStartEngine → WinMain-equivalent flow), not just the platform stub.
//
// Master Prompt §14/§24: every entry point is defensive — no exceptions or
// JNI misuse may crash the process; errors are returned to Kotlin instead.

#include <jni.h>
#include <string>
#include <cstdio>
#include <cstring>
#include <android/log.h>
#include <sys/system_properties.h>
#include <sys/stat.h>
#include <dirent.h>

#include "../android/AndroidMain.h"

#define LOG_TAG "generals-jni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace {

std::string sysProp(const char* key, const char* fallback = "?") {
    char value[PROP_VALUE_MAX] = {0};
    int len = __system_property_get(key, value);
    return len > 0 ? std::string(value) : std::string(fallback);
}

/** Readable-directory check for a path the engine will chdir() into. */
bool readableDir(const std::string& path) {
    if (path.empty()) return false;
    struct stat st{};
    if (::stat(path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) return false;
    DIR* d = ::opendir(path.c_str());
    if (d == nullptr) return false;
    ::closedir(d);
    return true;
}

std::string jStringToStd(JNIEnv* env, jstring s) {
    if (env == nullptr || s == nullptr) return std::string();
    const char* c = env->GetStringUTFChars(s, nullptr);
    if (c == nullptr) return std::string();
    std::string out(c);
    env->ReleaseStringUTFChars(s, c);
    return out;
}

} // namespace

extern "C" {

JNIEXPORT jstring JNICALL
Java_com_obsifox_generals_nativelib_NativeBridge_engineVersion(JNIEnv* env, jobject /*thiz*/) {
    const char* v = "?";
    try { v = generals::engineVersionString(); }
    catch (...) { v = "engine (version probe failed)"; }
    return env->NewStringUTF(v);
}

JNIEXPORT jboolean JNICALL
Java_com_obsifox_generals_nativelib_NativeBridge_nativeStartEngine(
        JNIEnv* env, jobject /*thiz*/, jobjectArray paths) {
    if (env == nullptr || paths == nullptr) return JNI_FALSE;
    const jsize count = env->GetArrayLength(paths);
    if (count < 2) return JNI_FALSE;

    const std::string dataDir = jStringToStd(
        env, static_cast<jstring>(env->GetObjectArrayElement(paths, 0)));
    const std::string internalDir = jStringToStd(
        env, static_cast<jstring>(env->GetObjectArrayElement(paths, 1)));

    if (!readableDir(dataDir)) {
        LOGW("nativeStartEngine: game data dir not readable: %s", dataDir.c_str());
        return JNI_FALSE;
    }
    if (internalDir.empty()) {
        LOGW("nativeStartEngine: internal dir missing");
        return JNI_FALSE;
    }

    LOGI("nativeStartEngine: booting REAL engine with data=%s", dataDir.c_str());
    try {
        const bool ok = generals::androidStartEngine(dataDir);
        LOGI("nativeStartEngine: androidStartEngine -> %s", ok ? "spawned" : "refused");
        return ok ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOGW("nativeStartEngine exception: %s", e.what());
        return JNI_FALSE;
    } catch (...) {
        LOGW("nativeStartEngine exception: unknown");
        return JNI_FALSE;
    }
}

JNIEXPORT void JNICALL
Java_com_obsifox_generals_nativelib_NativeBridge_nativeShutdownEngine(JNIEnv* env, jobject /*thiz*/) {
    try {
        generals::androidShutdownEngine();
    } catch (...) {
        // never crash on shutdown (§14)
    }
}

JNIEXPORT jstring JNICALL
Java_com_obsifox_generals_nativelib_NativeBridge_nativeEngineStatus(JNIEnv* env, jobject /*thiz*/) {
    char buf[512] = {0};
    int n = 0;
    try {
        n = generals::androidEngineStatus(buf, sizeof(buf));
    } catch (...) {
        n = snprintf(buf, sizeof(buf), "phase=unknown detail=status probe failed");
    }
    if (n <= 0) {
        strncpy(buf, "phase=unknown detail=no status", sizeof(buf) - 1);
    }
    return env->NewStringUTF(buf);
}

JNIEXPORT jstring JNICALL
Java_com_obsifox_generals_nativelib_NativeBridge_nativeDeviceProfile(JNIEnv* env, jobject /*thiz*/) {
    // Device capability profile (Master Prompt §25)
    std::string profile =
        "abi=" + sysProp("ro.product.cpu.abi") +
        " soc=" + sysProp("ro.board.platform") +
        " memMB=" + sysProp("ro.boot.memory_mb", "0") +
        " renderer=gles3";
    return env->NewStringUTF(profile.c_str());
}

} // extern "C"
