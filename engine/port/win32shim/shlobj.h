// shlobj.h shim — shell folder APIs replaced by app storage on Android.
#pragma once
#include "windows.h"
#define CSIDL_DESKTOP 0
#define CSIDL_PERSONAL 5
#define CSIDL_LOCAL_APPDATA 28
#define SHCreateDirectory_shim_unused 1
inline HRESULT SHGetFolderPathA(void* hwnd, int folder, HANDLE token, DWORD flags, char* path) {
    if (path) path[0] = 0;
    return E_FAIL;
}
#define SHGetFolderPath SHGetFolderPathA
