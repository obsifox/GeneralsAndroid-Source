# engine/port/CMakeLists.txt — Android port target, included from the pinned
# upstream root CMakeLists (see engine/port/patches/0001-*.patch) when the
# ANDROID platform is configured. Host (Linux) builds are also supported for
# fast local iteration of the same sources.
# GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.

# GENERALS_PORT_DIR is set by the root patch (or cache) to this directory.

# ——— gamespy bionic compat (configure-time, idempotent) ———
# The populated SDK dir is visible here because the upstream tree (which
# FetchContent_MakeAvailable's gamespy) is a sibling added BEFORE this file.
if(ANDROID AND DEFINED gamespy_SOURCE_DIR AND EXISTS "${gamespy_SOURCE_DIR}/src")
    set(SDK_DIR "${gamespy_SOURCE_DIR}")
    include("${CMAKE_CURRENT_LIST_DIR}/patches/gamespy-bionic.cmake")
    unset(SDK_DIR)
endif()

add_library(generals_android SHARED
    "${GENERALS_PORT_DIR}/win32shim/win32shim.cpp"
    "${GENERALS_PORT_DIR}/android/AndroidMain.cpp"
    "${GENERALS_PORT_DIR}/android/AndroidGameEngine.cpp"
)

if(ANDROID)
    target_sources(generals_android PRIVATE
        "${GENERALS_PORT_DIR}/jni/generals_jni.cpp"
    )
endif()

# Version headers, generated the same way GeneralsMD/Code/Main does.
set(GENERALS_PORT_BIN "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(WRITE "${GENERALS_PORT_BIN}/BuildVersion.h"
"#pragma once
#define VERSION_MAJOR 1
#define VERSION_MINOR 4
#define VERSION_BUILDNUM 601
")
file(WRITE "${GENERALS_PORT_BIN}/GeneratedVersion.h"
"#pragma once
#define VERSION_LOCALBUILDNUM 0
#define VERSION_BUILDUSER \"obsifox-android\"
#define VERSION_BUILDLOC \"android\"
")

target_include_directories(generals_android PRIVATE
    "${GENERALS_PORT_DIR}"
    "${GENERALS_PORT_BIN}"
    "${GENERALS_PORT_DIR}/android"
)

# The game targets carry the engine sources + includes; linking them compiles
# the full engine into libgenerals.so.
target_link_libraries(generals_android PRIVATE
    zi_always               # RTS_ZEROHOUR=1 + corei_always chain
    z_gameengine            # ZH + core engine sources (STATIC)
    z_gameenginedevice      # ZH + core device sources (STATIC)
    deps_bink
    deps_miles
    deps_utility
)

if(TARGET libzlib)
    target_link_libraries(generals_android PRIVATE libzlib)
endif()

if(ANDROID)
    find_library(log-lib log)
    find_library(android-lib android)
    find_library(egl-lib EGL)
    find_library(gles-lib GLESv3)
    find_library(sles-lib OpenSLES)
    target_link_libraries(generals_android PRIVATE
        ${log-lib} ${android-lib} ${egl-lib} ${gles-lib} ${sles-lib}
    )
endif()

# Visibility: JNI entry points + the port bootstrap API only.
target_compile_options(generals_android PRIVATE
    -fvisibility=hidden
    -Wno-unused-parameter
    -Wno-unknown-pragmas
)
set_target_properties(generals_android PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    OUTPUT_NAME "generals"
)

# Gamespy SDK: force its POSIX branch — its Win32 branch needs real Winsock,
# which is absent here. -U after -D undefines at the per-target level.
set(_gs_targets
    gschat gscommon gscdkey gshttp gsgp gsgstats gsgt2
    gsnatneg gspeer gspinger gspt gsqr gsqr2
)
foreach(_t IN LISTS _gs_targets)
    if(TARGET "${_t}")
        target_compile_options("${_t}" PRIVATE -U_WIN32 -UWIN32)
    endif()
endforeach()
