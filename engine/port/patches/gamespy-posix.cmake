# gamespy-posix.cmake — flip the GameSpy SDK platform gates to the POSIX
# branch when building the obsifox Android port (GENERALS_PORT).
# GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
file(GLOB _gs_headers "${SDK_DIR}/include/gamespy/gsplatform*.h")
foreach(_h IN LISTS _gs_headers)
    file(READ "${_h}" _content)
    string(REPLACE "#ifdef _WIN32" "#if defined(_WIN32) && !defined(GENERALS_PORT)" _content "${_content}")
    string(REPLACE "#if defined(_WIN32)" "#if defined(_WIN32) && !defined(GENERALS_PORT)" _content "${_content}")
    string(REPLACE "#elif defined(_WIN32)" "#elif defined(_WIN32) && !defined(GENERALS_PORT)" _content "${_content}")
    file(WRITE "${_h}" "${_content}")
endforeach()

# obsifox port: bionic (Android libc) has NO POSIX thread-cancellation API at
# all — pthread_cancel/setcancelstate/setcanceltype/testcancel do not exist.
# GameSpy's linux threading layer (gsthreadlinux.c) uses them; on Android the
# cancellation calls degrade to no-ops (chat/peer threads shut down with the
# process, which is the only cancellation semantics the mobile port needs).
set(_gs_bionic_compat [[
#if defined(__ANDROID__)
/* obsifox port: bionic has no POSIX thread-cancellation API (see
   engine/port/patches/gamespy-posix.cmake). Calls become no-ops. */
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
        message(STATUS "gamespy-posix: bionic compat prepended to ${_s}")
    endif()
endforeach()
