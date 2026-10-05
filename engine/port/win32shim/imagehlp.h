// imagehlp.h shim — crash-dump helpers do not exist on Android; the crash
// path is inert on mobile (Android tombstones instead). Only the dbghelp
// loader typedefs are needed for compilation.
#pragma once
#include "windows.h"
typedef const char* PCSTR;
typedef char* PSTR;
typedef struct _ADDRESS64 {
    DWORD64 Offset;
    WORD Segment;
    WORD Mode;
} ADDRESS64, *LPADDRESS64;
typedef BOOL (*PREAD_PROCESS_MEMORY_ROUTINE)(HANDLE, DWORD, void*, DWORD, SIZE_T*);
typedef LPVOID (*PFUNCTION_TABLE_ACCESS_ROUTINE)(HANDLE, DWORD);
typedef DWORD (*PGET_MODULE_BASE_ROUTINE)(HANDLE, DWORD);
typedef BOOL (*PTRANSLATE_ADDRESS_ROUTINE)(LPADDRESS64, LPADDRESS64, LPVOID);
