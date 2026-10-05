# Generals Android — native layer
# When GENERALS_ENGINE_ROOT points at the fetched+patched pinned upstream
# (engine/fetch_upstream.sh output), the REAL engine is compiled into
# libgenerals.so via the port layer (engine/port). Otherwise this builds the
# standalone platform shell (legacy phase-2 layout).
# GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.

cmake_minimum_required(VERSION 3.22)
project(generals LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED GENERALS_BUILD_PLATFORM_ANDROID)
    set(GENERALS_BUILD_PLATFORM_ANDROID ON)
endif()

if(NOT GENERALS_ENGINE_ROOT)
    # ——— Legacy standalone shell (no engine) ———
    add_library(generals SHARED
        generals_jni.cpp
        platform/AndroidDevice.cpp
        platform/TouchMapper.cpp
    )
    target_include_directories(generals PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/platform
    )
    find_library(log-lib log)
    find_library(android-lib android)
    find_library(egl-lib EGL)
    find_library(gles-lib GLESv3)
    target_link_libraries(generals
        ${log-lib}
        ${android-lib}
        ${egl-lib}
        ${gles-lib}
    )
    return()
endif()

# ——— Real engine build ———
# The upstream root CMakeLists (patched, see engine/port/patches) switches the
# whole tree onto the POSIX-backed Win32 shim (engine/port/win32shim): _WIN32
# gates + force-included portsystem.h. Those directory-scope settings are
# replayed HERE so both the upstream tree and the port layer (a sibling scope)
# receive identical flags (the patched upstream root does the same internally).
message(STATUS "Linking upstream engine from ${GENERALS_ENGINE_ROOT}")
add_compile_definitions(_WIN32 WIN32 GENERALS_PORT)
include_directories(
    "${GENERALS_ENGINE_ROOT}/../port/win32shim"
    "${GENERALS_ENGINE_ROOT}/Dependencies/Utility"
    "${GENERALS_ENGINE_ROOT}/Dependencies/DbgHelp"
)
add_compile_options("-include" "${GENERALS_ENGINE_ROOT}/../port/win32shim/portsystem.h")
add_subdirectory(${GENERALS_ENGINE_ROOT} generals-engine)

# The Android port layer (engine/port) defines `generals_android`
# (OUTPUT_NAME "generals") — the JNI glue + win32shim + engine bootstrap.
# It is added HERE (top-level scope) because AGP's `targets` spec can only
# address targets visible at the project root it configures.
set(GENERALS_PORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../engine/port")
message(STATUS "Adding Android port layer from ${GENERALS_PORT_DIR}")
add_subdirectory(${GENERALS_PORT_DIR} generals-android-port)
