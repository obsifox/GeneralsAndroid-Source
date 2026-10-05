// snmp.h shim — referenced by PreRTS.h and the GameSpy staging-room code;
// only the memory helpers and IN/OUT context are needed for compilation.
#pragma once
#include "windows.h"
inline void* SnmpUtilMemAlloc(UINT bytes) { return malloc(bytes); }
inline void SnmpUtilMemFree(void* p) { free(p); }
inline void* SnmpUtilMemReAlloc(void* p, UINT bytes) { return realloc(p, bytes); }
typedef void* SnmpUtilMemFreePtr_shim_unused;
