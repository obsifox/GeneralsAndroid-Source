// generals_jni.cpp — JNI entry points for the Persian Android port.
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
//
// Phase status (docs/IMPLEMENTATION_PLAN.md):
//   [x] platform bootstrap + device profile reporting
//   [ ] upstream engine link (phases 3-5, seam in CMakeLists.txt)
//
// Never crashes on bad input (Master Prompt §14, §24): every JNI function is
// defensive and returns false/null instead of aborting the process.

#include <jni.h>
#include <string>
#include <android/log.h>
#include <sys/system_properties.h>

#include "platform/AndroidDevice.h"
#include "platform/TouchMapper.h"

#define LOG_TAG "generals-jni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace {

constexpr const char* kEngineVersion = "0.1.0-alpha (platform layer; engine link phase 3-5)";

std::string sysProp(const char* key, const char* fallback = "?") {
    char value[PROP_VALUE_MAX] = {0};
    int len = __system_property_get(key, value);
    return len > 0 ? std::string(value) : std::string(fallback);
}

} // namespace

extern "C" {

JNIEXPORT jstring JNICALL
Java_com_obsifox_generals_nativelib_NativeBridge_engineVersion(JNIEnv* env, jobject /*thiz*/) {
    return env->NewStringUTF(kEngineVersion);
}

JNIEXPORT jboolean JNICALL
Java_com_obsifox_generals_nativelib_NativeBridge_nativeStartEngine(
        JNIEnv* env, jobject /*thiz*/, jobjectArray paths) {
    if (paths == nullptr) return JNI_FALSE;
    const jsize count = env->GetArrayLength(paths);
    if (count < 2) return JNI_FALSE;

    jstring dataJ = static_cast<jstring>(env->GetObjectArrayElement(paths, 0));
    jstring internalJ = static_cast<jstring>(env->GetObjectArrayElement(paths, 1));
    if (dataJ == nullptr || internalJ == nullptr) return JNI_FALSE;

    const char* dataC = env->GetStringUTFChars(dataJ, nullptr);
    const char* internalC = env->GetStringUTFChars(internalJ, nullptr);
    if (dataC == nullptr || internalC == nullptr) {
        if (dataC) env->ReleaseStringUTFChars(dataJ, dataC);
        if (internalC) env->ReleaseStringUTFChars(internalJ, internalC);
        return JNI_FALSE;
    }

    const bool ok = generals::AndroidDevice::bootstrap(
        std::string(dataC), std::string(internalC));

    LOGI("nativeStartEngine: data=%s internal=%s -> %s",
         dataC, internalC, ok ? "ok" : "invalid");

    env->ReleaseStringUTFChars(dataJ, dataC);
    env->ReleaseStringUTFChars(internalJ, internalC);
    return ok ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_obsifox_generals_nativelib_NativeBridge_nativeShutdownEngine(JNIEnv* /*env*/, jobject /*thiz*/) {
    generals::AndroidDevice::shutdown();
}

JNIEXPORT jstring JNICALL
Java_com_obsifox_generals_nativelib_NativeBridge_nativeDeviceProfile(JNIEnv* env, jobject /*thiz*/) {
    // Device capability profile (Master Prompt §25)
    std::string profile =
        "abi=" + sysProp("ro.product.cpu.abi") +
        " soc=" + sysProp("ro.board.platform") +
        " memMB=" + sysProp("ro.boot.memory_mb", "0") +
        " renderer=" + generals::AndroidDevice::preferredRenderer();
    return env->NewStringUTF(profile.c_str());
}

} // extern "C"
