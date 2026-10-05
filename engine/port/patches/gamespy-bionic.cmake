# gamespy-bionic.cmake — bionic/Android compatibility for the GameSpy SDK,
# applied at CONFIGURE time from engine/port/CMakeLists.txt (the SDK's
# PATCH_COMMAND hook proved unreliable across FetchContent population paths,
# and configure-time patching is guaranteed to run before any gs* target
# compiles). Idempotent: safe when the PATCH_COMMAND already patched.
# GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
#
# Expects SDK_DIR (the populated gamespy source dir) in scope.

if(NOT DEFINED SDK_DIR OR NOT EXISTS "${SDK_DIR}/src")
    return()
endif()

# 1) platform gates → POSIX branch even with GENERALS_PORT's _WIN32 defines
#    (engine/port/CMakeLists.txt also -U_WIN32's the gs* targets; both ways
#    are kept because the SDK mixes header-level and source-level gates).
file(GLOB _gs_headers "${SDK_DIR}/include/gamespy/gsplatform*.h")
foreach(_h IN LISTS _gs_headers)
    if(NOT EXISTS "${_h}")
        continue()
    endif()
    file(READ "${_h}" _content)
    if(_content MATCHES "GENERALS_PORT")
        continue() # already flipped
    endif()
    string(REPLACE "#ifdef _WIN32" "#if defined(_WIN32) && !defined(GENERALS_PORT)" _content "${_content}")
    string(REPLACE "#if defined(_WIN32)" "#if defined(_WIN32) && !defined(GENERALS_PORT)" _content "${_content}")
    string(REPLACE "#elif defined(_WIN32)" "#elif defined(_WIN32) && !defined(GENERALS_PORT)" _content "${_content}")
    file(WRITE "${_h}" "${_content}")
    message(STATUS "gamespy-bionic: platform gates flipped in ${_h}")
endforeach()

# 2) bionic has NO POSIX thread-cancellation API at all — pthread_cancel /
#    setcancelstate / setcanceltype / testcancel do not exist. GameSpy's
#    linux threading layer (gsthreadlinux.c) uses them; on Android the
#    cancellation calls degrade to no-ops (chat/peer threads shut down with
#    the process — the only cancellation semantics the mobile port needs).
set(_gs_bionic_compat [[
#if defined(__ANDROID__)
/* obsifox port: bionic has no POSIX thread-cancellation API (see
   engine/port/patches/gamespy-bionic.cmake). Calls become no-ops. */
#include <pthread.h>
#include <errno.h>
static inline int gs_port_pthread_cancel(pthread_t t) { (void)t; return ESRCH; }
#define pthread_cancel(t) gs_port_pthread_cancel(t)
static inline int gs_port_pthread_setcancelstate(int state, int *oldstate) {
    (void)state; if (oldstate) *oldstate = 0; return 0;
}
#define pthread_setcancelstate(state, oldstate) gs_port_pthread_setcancelstate(state, oldstate)
static inline int gs_port_pthread_setcanceltype(int type, int *oldtype) {
    (void)type; if (oldtype) *oldtype = 0; return 0;
}
#define pthread_setcanceltype(type, oldtype) gs_port_pthread_setcanceltype(type, oldtype)
static inline void gs_port_pthread_testcancel(void) {}
#define pthread_testcancel() gs_port_pthread_testcancel()
#endif
]])

file(GLOB_RECURSE _gs_thread_sources "${SDK_DIR}/src/*.c" "${SDK_DIR}/src/*.h")
foreach(_s IN LISTS _gs_thread_sources)
    file(READ "${_s}" _content)
    string(FIND "${_content}" "pthread_cancel" _idx)
    if(_idx GREATER -1 AND NOT _content MATCHES "gs_port_pthread_cancel")
        file(WRITE "${_s}" "${_gs_bionic_compat}\n${_content}")
        message(STATUS "gamespy-bionic: thread-cancel compat prepended to ${_s}")
    endif()
endforeach()
