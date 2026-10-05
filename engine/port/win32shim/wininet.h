// wininet.h shim — HTTP(S) download layer is inert on Android for now;
// the engine's online components are disabled on mobile (LAN-only alpha).
#pragma once
#include "windows.h"
typedef void* HINTERNET_shim_pad;
