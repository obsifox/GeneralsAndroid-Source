// shellapi.h shim — shell operations are Kotlin-side on Android.
#pragma once
#include "windows.h"
#define SW_SHOWNOACTIVATE 4
inline HINSTANCE ShellExecuteA(HWND, const char*, const char*, const char*, const char*, int) { return nullptr; }
#define ShellExecute ShellExecuteA
