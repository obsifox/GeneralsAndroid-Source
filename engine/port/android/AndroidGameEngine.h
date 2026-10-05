// AndroidGameEngine.h — engine subclass for the Android port.
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
//
// Reuses Win32GameEngine wholesale: the Win32 API surface it touches is
// provided by engine/port/win32shim (POSIX-backed). Only the Windows message
// pump is neutralised — on Android, lifecycle events arrive from the Kotlin
// side instead (Master Prompt §17).

#pragma once

#include "Win32Device/Common/Win32GameEngine.h"

namespace generals {

class AndroidGameEngine final : public Win32GameEngine
{
public:
    AndroidGameEngine();
    virtual ~AndroidGameEngine() override;

    virtual void serviceWindowsOS() override;  // no-op: no Windows message pump

    // The SuperHackers upstream: engine reports; kept minimal for the headless boot.
    virtual void init() override;
};

} // namespace generals
