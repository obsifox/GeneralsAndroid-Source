// portsystem.h — force-included system header for the Android port build.
// Applied via add_compile_options(-include ...) in the root CMake patch.
// Every TU gets: the standard headers the Win32-era code assumed, the
// windows.h/winsock2 shims (POSIX-backed), and the upstream Utility compat
// shims for strlcpy-family helpers (compat.h itself is _WIN32-gated).
// GPLv3-or-later. obsifox/GeneralsAndroid-Source.
#pragma once

#ifdef __cplusplus
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cassert>
#include <cwctype>
#include <cwchar>
#include <algorithm>
#include <new>
#else
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#endif

// Win32-era scalar spellings — MSVC keyword emulation via macro so that
// `unsigned __int64` parses (a typedef-name cannot take unsigned in C++).
#define __int64 long long
#define _int64 long long
typedef long long sint64;
typedef unsigned long long uint64;

// Port Win32 surface (POSIX-backed) — every TU, C and C++.
#include "windows.h"
#include "winsock2.h"

#ifdef __cplusplus
// Upstream Utility compat helpers (C++ only — third-party C sources define
// their own strlwr-family helpers and would collide).
#include <Utility/mem_compat.h>
#include <Utility/string_compat.h>
#include <Utility/tchar_compat.h>
#include <Utility/wchar_compat.h>

#define _strdup strdup
#include <strings.h>
#ifndef _stricmp
inline int _stricmp(const char* a, const char* b) { return strcasecmp(a, b); }
#endif
#ifndef _strnicmp
inline int _strnicmp(const char* a, const char* b, size_t n) { return strncasecmp(a, b, n); }
#endif
#define _isnan(x) isnan(x)
#define _MAX_DRIVE 3
#define _MAX_DIR 256
#define _MAX_FNAME 256
#define _MAX_EXT 64
#define _MAX_PATH 260

#ifndef __forceinline
#define __forceinline inline
#endif
#endif
