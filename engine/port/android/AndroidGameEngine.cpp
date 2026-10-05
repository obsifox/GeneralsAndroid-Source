// AndroidGameEngine.cpp — engine subclass for the Android port.
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.

#include "AndroidGameEngine.h"

#include "android_log_shim.h"

#define TAG "generals-engine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

namespace generals {

AndroidGameEngine::AndroidGameEngine()
{
    LOGI("AndroidGameEngine: constructed");
}

AndroidGameEngine::~AndroidGameEngine()
{
    LOGI("AndroidGameEngine: destroyed");
}

void AndroidGameEngine::serviceWindowsOS()
{
    // §35 workaround: the Win32 message pump does not exist on Android.
    // Lifecycle/input arrive from the Kotlin side; nothing to service here.
    // LAN updates that Win32GameEngine's alt-tab path performs are handled by
    // the normal engine update cadence on mobile.
}

void AndroidGameEngine::init()
{
    // Extend, don't replace: the base init wires every subsystem; device
    // failures (renderer/audio) surface through their own status channels.
    Win32GameEngine::init();
    LOGI("AndroidGameEngine: init complete");
}

} // namespace generals
