// AndroidMain.h — public bootstrap API of the engine port layer.
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
#pragma once

#include <string>

namespace generals {

/** Boots the real engine on a worker thread (WinMain-equivalent flow).
    gameDataDir: directory holding the user-imported game files (BIG/INI). */
bool androidStartEngine(const std::string& gameDataDir);

/** Requests engine shutdown and joins the worker thread. */
void androidShutdownEngine();

/** One-line engine status (phase + detail) for the diagnostics UI. */
int androidEngineStatus(char* out, int outLen);

/** Human-readable engine version string (BuildVersion + port marker). */
const char* engineVersionString();

} // namespace generals
