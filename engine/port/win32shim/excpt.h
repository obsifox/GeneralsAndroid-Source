// excpt.h shim — SEH does not exist on Android (tombstones instead); the
// exception-pointer type is declared so inert crash-path headers compile.
#pragma once
typedef struct _EXCEPTION_RECORD64_shim {
    DWORD ExceptionCode;
    void* ExceptionAddress;
} _EXCEPTION_RECORD64;
typedef struct _EXCEPTION_POINTERS {
    void* ExceptionRecord;
    void* ContextRecord;
} _EXCEPTION_POINTERS, *LPTOP_LEVEL_EXCEPTION_FILTER_shim_unused;
typedef LONG (*PTOPLEVEL_EXCEPTION_FILTER)(void*);
#define LPTOP_LEVEL_EXCEPTION_FILTER PTOPLEVEL_EXCEPTION_FILTER
