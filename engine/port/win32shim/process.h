// process.h shim — _beginthreadex on pthreads.
#pragma once
#include "windows.h"
uintptr_t _beginthread(void (*start)(void*), void* stack, void* arg);
inline uintptr_t _beginthreadex(void* sec, unsigned stack, unsigned (*start)(void*), void* arg, unsigned initflag, unsigned* tid) {
    return reinterpret_cast<uintptr_t>(CreateThread(sec, stack, reinterpret_cast<LPTHREAD_START_ROUTINE>(start), arg, initflag, nullptr));
}
inline void _endthreadex(unsigned rc) { pthread_exit(reinterpret_cast<void*>(static_cast<uintptr_t>(rc))); }
#define _exit exit
