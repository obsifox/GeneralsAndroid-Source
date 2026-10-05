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
# The upstream root CMakeLists (patched, see engine/port/patches) activates
# the port block when ANDROID is set, and adds engine/port as the
# `generals_android` target (OUTPUT_NAME "generals") with the JNI glue.
message(STATUS "Linking upstream engine from ${GENERALS_ENGINE_ROOT}")
add_subdirectory(${GENERALS_ENGINE_ROOT} generals-engine EXCLUDE_FROM_ALL)
