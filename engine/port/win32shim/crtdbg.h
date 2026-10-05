// crtdbg.h shim — asserts route to the Android log.
#pragma once
#include "android_log_shim.h"
#define _CRT_ASSERT 0
#define _CRT_ERROR 1
#define _CRT_WARN 2
#define _ASSERT(expr) ((void)0)
#define _ASSERTE(expr) ((void)0)
#define _RPT0(t, msg) __android_log_print(ANDROID_LOG_WARN, "crt", "%s", msg)
#define _RPT1(t, fmt, a) __android_log_print(ANDROID_LOG_WARN, "crt", fmt, a)
#define _RPT2(t, fmt, a, b) __android_log_print(ANDROID_LOG_WARN, "crt", fmt, a, b)
#define _RPTF0(t, msg) _RPT0(t, msg)
#define _RPTF1(t, fmt, a) _RPT1(t, fmt, a)
#define _RPTF2(t, fmt, a, b) _RPT2(t, fmt, a, b)
#define _CrtDbgReport(...) (0)
#define _CrtSetDbgFlag(f) (0)
#define _CrtSetReportMode(t, m) (0)
#define _CRTDBG_REPORT_MODE (-1)
#define _CRTDBG_MODE_DEBUG 0
#define _CRTDBG_CHECK_ALWAYS_DF 0
#define _CRTDBG_ALLOC_MEM_DF 0
