// crtcompat.h — MUST be included before any C/C++ standard header on Android.
// portsystem.h (force-included first in every TU) pulls this in at line 1.
//
// Why: the port defines _WIN32 so the Win32-era engine code keeps compiling
// against the POSIX-backed shim. libc++ (clang's C++ stdlib) sees _WIN32 too
// and takes its MSVCRT-LIKE branches — e.g. <new> calls ::_aligned_malloc /
// ::_aligned_free, which bionic does not have. We provide those surfaces as
// macros BEFORE libc++ is ever included (§35: workaround documented here).
//
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
#ifndef GENERALS_PORT_CRT_COMPAT_H
#define GENERALS_PORT_CRT_COMPAT_H

#if defined(__ANDROID__) && defined(_WIN32)

#include <malloc.h>
#include <stdlib.h>

/* bionic has memalign on every API level (aligned_alloc needs API 28+;
   minSdk is 24). libc++'s <new> windows branch calls ::_aligned_malloc. */
#define _aligned_malloc(size, alignment) \
    memalign(((alignment) < sizeof(void*)) ? sizeof(void*) : (alignment), (size))
#define _aligned_free(ptr) free(ptr)

#endif /* __ANDROID__ && _WIN32 */

#endif /* GENERALS_PORT_CRT_COMPAT_H */
