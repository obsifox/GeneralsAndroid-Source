// android_log_shim.h — logging that works on Android AND on the Linux host
// iteration build (port/win32shim must compile off-device too).
#pragma once

#if defined(__ANDROID__)
#include "android_log_shim.h"
#else
#include <stdio.h>
#define ANDROID_LOG_DEBUG 3
#define ANDROID_LOG_INFO 4
#define ANDROID_LOG_WARN 5
#define ANDROID_LOG_ERROR 6
#define __android_log_print(prio, tag, ...) \
    do { fprintf(stderr, "[%s] ", tag); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#endif
