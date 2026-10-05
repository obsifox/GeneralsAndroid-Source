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
