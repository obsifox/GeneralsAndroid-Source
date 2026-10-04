# Generals Android — native platform library (ARM64)
# Phase 3-4 scaffold: compiles the Android platform layer standalone now and
# exposes a clean seam for the pinned upstream engine (docs/ARCHITECTURE_DECISION.md).
# When GENERALS_ENGINE_ROOT is set (engine/fetch_upstream.sh output), the engine
# sources are added to the same target — no #ifdef pollution reaches the core.

cmake_minimum_required(VERSION 3.22)
project(generals LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED GENERALS_BUILD_PLATFORM_ANDROID)
    set(GENERALS_BUILD_PLATFORM_ANDROID ON)
endif()

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

# —— Upstream engine seam (activated by CI/experimental builds) ——
# The engine is fetched pinned by engine/fetch_upstream.sh into
# <repo>/engine/upstream. This block is the ONLY place that references it.
set(GENERALS_ENGINE_ROOT "" CACHE PATH "Path to pinned upstream engine source")
if(GENERALS_ENGINE_ROOT AND EXISTS "${GENERALS_ENGINE_ROOT}/CMakeLists.txt")
    message(STATUS "Linking upstream engine from ${GENERALS_ENGINE_ROOT}")
    add_subdirectory(${GENERALS_ENGINE_ROOT} generals-engine EXCLUDE_FROM_ALL)
    target_link_libraries(generals generals_core)
    target_compile_definitions(generals PRIVATE GENERALS_HAVE_UPSTREAM=1)
endif()

# Master Prompt §35: every workaround is documented at the site of the hack.
target_compile_options(generals PRIVATE
    -Wall -Wextra
    -Wno-unused-parameter
)
