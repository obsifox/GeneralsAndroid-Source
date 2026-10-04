// AndroidDevice.cpp — Android platform device layer implementation.
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
#include "AndroidDevice.h"

#include <dirent.h>
#include <sys/stat.h>
#include <android/log.h>

#define LOG_TAG "generals-device"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace generals {

bool AndroidDevice::s_bootstrapped = false;
std::string AndroidDevice::s_gameDataDir;
std::string AndroidDevice::s_internalDir;

bool AndroidDevice::bootstrap(const std::string& gameDataDir,
                              const std::string& internalFilesDir) {
    s_bootstrapped = false;
    s_gameDataDir.clear();
    s_internalDir.clear();

    if (gameDataDir.empty() || internalFilesDir.empty()) return false;

    struct stat st{};
    if (stat(gameDataDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
        LOGW("game data dir not accessible: %s", gameDataDir.c_str());
        return false; // useful error upstream in Kotlin; never crash (§14)
    }

    // Security (§34): treat imported game files as untrusted input.
    // We only validate readability here; archive parsing lives behind the
    // engine seam and will enforce path-traversal guards when linked (phase 3+).
    DIR* dir = opendir(gameDataDir.c_str());
    if (dir == nullptr) {
        LOGW("opendir failed: %s", gameDataDir.c_str());
        return false;
    }
    // Count .big archives as a smoke check (full validation is Kotlin-side).
    int bigCount = 0;
    while (const dirent* entry = readdir(dir)) {
        const std::string name = entry->d_name;
        if (name.length() > 4 && name.substr(name.length() - 4) == ".big") {
            ++bigCount;
        }
    }
    closedir(dir);

    s_gameDataDir = gameDataDir;
    s_internalDir = internalFilesDir;
    s_bootstrapped = true;
    LOGI("bootstrap ok: %d .big archives visible", bigCount);
    return true;
}

void AndroidDevice::shutdown() {
    s_bootstrapped = false;
    LOGI("shutdown");
}

std::string AndroidDevice::preferredRenderer() {
    // §5: auto-selection heuristic (refined in phase 5 with real probing):
    // Vulkan where advertised, GLES3 as universal fallback, safe mode last.
    return "gles3";
}

} // namespace generals
