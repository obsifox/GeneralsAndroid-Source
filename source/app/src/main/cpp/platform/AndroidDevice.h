// AndroidDevice.h — Android platform device layer (Master Prompt §4).
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
//
// This file is the future home of the engine-facing device implementations
// (file system, timing, threads) that upstream currently only provides for
// Win32 (see docs/ARCHITECTURE_DECISION.md §3-4). Kept deliberately small and
// isolated: the engine core must never contain Android conditionals.
#pragma once

#include <string>

namespace generals {

class AndroidDevice {
public:
    /// Validates the imported game-data directory and prepares internal state.
    /// Returns false (never crashes) when data is unusable — the Kotlin side
    /// then shows a localized, actionable message (Master Prompt §14).
    static bool bootstrap(const std::string& gameDataDir,
                          const std::string& internalFilesDir);

    static void shutdown();

    /// Best renderer for this device: "vulkan" | "gles3" | "safe" (§5).
    /// Query-based, never hard-coded per model.
    static std::string preferredRenderer();

private:
    static bool s_bootstrapped;
    static std::string s_gameDataDir;
    static std::string s_internalDir;
};

} // namespace generals
