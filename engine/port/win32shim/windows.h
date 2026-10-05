// windows.h — Win32 compatibility shim for the Android port of the
// Generals engine (obsifox/GeneralsAndroid-Source, GPLv3-or-later).
//
// This header SHADOWS the real windows.h on Android/POSIX via include-path
// precedence (engine/port/win32shim comes first). No upstream file is edited.
//
// Layout (dependency order): calling-convention macros → C base types →
// Win32 scalar types + GUID → handles → structs → function declarations.
// D3D/DInput/Draw surfaces are NOT shimmed semantically — they compile
// against the min-dx8-sdk headers and fail fast at runtime (B1/B2 in
// docs/KNOWN-BLOCKERS.md).

#pragma once

#if defined(_MSC_VER) || defined(__MINGW32__)
#error "win32shim must never be used on a real Windows compiler"
#endif

// ---------------------------------------------------------------------------
// Calling-convention / attribute macros (no-ops on GCC/Clang)
// ---------------------------------------------------------------------------
#define WINAPI
#define APIENTRY
#define CALLBACK
#define WINAPIV
#define __stdcall
#define __cdecl
#define __fastcall
#define STDMETHODCALLTYPE
#define STDAPICALLTYPE
#define FAR
#define NEAR
#define PASCAL
#define CDECL
#define _far
#define _near
#define _pascal

// ---------------------------------------------------------------------------
// C base types
// ---------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

// ---------------------------------------------------------------------------
// Win32 scalar types (interlocked with WWLib/bittype.h — identical spelling)
// ---------------------------------------------------------------------------
#ifndef GENERALS_PORT_WIN32_TYPES
#define GENERALS_PORT_WIN32_TYPES
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef unsigned int DWORD;
typedef int INT;
typedef long LONG;
typedef unsigned long ULONG;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef unsigned long long DWORD64;
typedef long long LONG64;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef const wchar_t* LPCWSTR;
typedef wchar_t* LPWSTR;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef unsigned char* LPBYTE;
typedef unsigned char* PBYTE;
typedef unsigned short* PWORD;
typedef unsigned int* PDWORD;
typedef unsigned int* PUINT;
typedef DWORD* LPDWORD;
typedef long* LPLONG;
typedef long HRESULT;
typedef size_t SIZE_T;
typedef long NTSTATUS;

typedef struct _GUID {
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;
typedef GUID* LPGUID;
typedef const GUID* LPCGUID;
#endif // GENERALS_PORT_WIN32_TYPES

typedef unsigned int MMRESULT;
typedef float FLOAT;
typedef double DOUBLE;
typedef void VOID;
typedef void* PVOID;
typedef intptr_t INT_PTR;
typedef uintptr_t UINT_PTR;
typedef long LONG_PTR;
typedef unsigned long ULONG_PTR;
typedef unsigned long DWORD_PTR;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define MAX_PATH 260

// ---------------------------------------------------------------------------
// HRESULT / error codes
// ---------------------------------------------------------------------------
#define S_OK ((HRESULT)0L)
#define S_FALSE ((HRESULT)1L)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_INVALIDARG ((HRESULT)0x80070057L)
#define E_POINTER ((HRESULT)0x80004003L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_HANDLE ((HRESULT)0x80070006L)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define HRESULT_FROM_WIN32(x) ((HRESULT)(x))

#define NO_ERROR 0L
#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_PATH_NOT_FOUND 3L
#define ERROR_ACCESS_DENIED 5L
#define ERROR_INVALID_HANDLE 6L
#define ERROR_NOT_ENOUGH_MEMORY 8L
#define ERROR_NO_MORE_FILES 18L
#define ERROR_HANDLE_EOF 38L
#define ERROR_ALREADY_EXISTS 183L
#define ERROR_IO_PENDING 997L
#define WAIT_OBJECT_0 0L
#define WAIT_TIMEOUT 258L
#define WAIT_FAILED 0xFFFFFFFFL
#define INFINITE 0xFFFFFFFFL

// File access
#define GENERIC_READ 0x80000000L
#define GENERIC_WRITE 0x40000000L
#define FILE_SHARE_READ 0x00000001L
#define FILE_SHARE_WRITE 0x00000002L
#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5
#define INVALID_HANDLE_VALUE ((HANDLE)(long long)-1)
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
#define INVALID_SET_FILE_POINTER 0xFFFFFFFFL
#define INVALID_FILE_SIZE 0xFFFFFFFFL
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010L
#define FILE_ATTRIBUTE_NORMAL 0x00000080L
#define FILE_ATTRIBUTE_READONLY 0x00000001L
#define FILE_ATTRIBUTE_HIDDEN 0x00000002L
#define FILE_ATTRIBUTE_SYSTEM 0x00000004L

// Window messages / UI constants (dev paths; the Android client never pumps
// real Windows messages — the Kotlin side drives lifecycle/input).
#define WM_QUIT 0x0012
#define WM_CLOSE 0x0010
#define WM_DESTROY 0x0002
#define WM_PAINT 0x000F
#define WM_ACTIVATE 0x0006
#define WM_SETCURSOR 0x0020
#define WM_USER 0x0400
#define PM_REMOVE 1
#define MB_OK 0x00000000L
#define MB_ICONERROR 0x00000010L
#define MB_ICONSTOP MB_ICONERROR
#define MB_ICONWARNING 0x00000030L
#define MB_ICONINFORMATION 0x00000040L
#define MB_ICONQUESTION 0x00000020L
#define MB_YESNO 0x00000004L
#define IDYES 6
#define IDNO 7
#define IDOK 1
#define IDCANCEL 2
#define SW_HIDE 0
#define SW_SHOWNORMAL 1
#define SW_SHOW 5
#define SW_RESTORE 9

// Memory / system info
#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_RELEASE 0x00008000
#define PAGE_READWRITE 0x04
#define PROCESSOR_ARCHITECTURE_AMD64 9
#define PROCESSOR_ARCHITECTURE_INTEL 0
#define PROCESSOR_ARCHITECTURE_ARM64 12

// Registry (INI-backed — see win32shim.cpp)
typedef void* HKEY;
typedef DWORD REGSAM;
#define HKEY_CLASSES_ROOT ((HKEY)0)
#define HKEY_CURRENT_CONFIG ((HKEY)1)
#define HKEY_CURRENT_USER ((HKEY)2)
#define HKEY_LOCAL_MACHINE ((HKEY)3)
#define HKEY_USERS ((HKEY)4)
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define KEY_ALL_ACCESS 0xF003F
#define REG_SZ 1
#define REG_DWORD 4
#define REG_BINARY 3
#define REG_OPTION_NON_VOLATILE 0x00000000L
#define REG_OPTION_VOLATILE 0x00000001L
#define VER_PLATFORM_WIN32s 0
#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT 2

// WinNT base vocabulary for the D3D8 SDK headers
#define CONST const
#define interface struct
#define DECLARE_HANDLE(name) typedef struct name##__ { int unused; } *name
#define ZeroMemory(dst, len) memset((dst), 0, (len))
#define FillMemory(dst, len, val) memset((dst), (val), (len))
#define CopyMemory(dst, src, len) memcpy((dst), (src), (len))
#define MoveMemory(dst, src, len) memmove((dst), (src), (len))
#define RtlZeroMemory ZeroMemory

// ---------------------------------------------------------------------------
// Handles
// ---------------------------------------------------------------------------
typedef struct Win32ShimObject_* HANDLE;
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *LPFILETIME;
typedef HANDLE HWND;
typedef HANDLE HDC;
typedef HANDLE HGLRC;
typedef HANDLE HINSTANCE;
typedef HANDLE HMODULE;
typedef HANDLE HCURSOR;
typedef HANDLE HICON;
typedef HANDLE HMENU;
typedef HANDLE HBRUSH;
typedef HANDLE HFONT;
typedef HANDLE HPALETTE;
typedef HANDLE HBITMAP;
typedef HANDLE HGDIOBJ;
typedef HANDLE HWAVEOUT;
typedef HWAVEOUT* LPHWAVEOUT;
DECLARE_HANDLE(HMONITOR); // d3d8.h skips its own when HMONITOR_DECLARED
#define HMONITOR_DECLARED
typedef HWAVEOUT* PHWAVEOUT;
typedef HANDLE HGLOBAL;

// ---------------------------------------------------------------------------
// Structs (all referenced by declarations below)
// ---------------------------------------------------------------------------
struct MSG {
    HWND hwnd;
    unsigned int message;
    unsigned int wParam;
    long lParam;
    DWORD time;
    long pt_x;
    long pt_y;
};
typedef struct MSG MSG, *LPMSG;

typedef struct POINT { long x; long y; } POINT, *LPPOINT;
typedef struct RECT { long left; long top; long right; long bottom; } RECT, *LPRECT;
typedef struct SIZE { long cx; long cy; } SIZE, *LPSIZE;

typedef struct SYSTEMTIME {
    WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;

typedef struct _OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    char szCSDVersion[128];
} OSVERSIONINFO, *LPOSVERSIONINFO;

typedef struct _SYSTEM_INFO {
    DWORD dwOemId;
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
    WORD wProcessorArchitecture;
} SYSTEM_INFO, *LPSYSTEM_INFO;

typedef struct _MEMORYSTATUSEX {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    unsigned long long ullTotalPhys;
    unsigned long long ullAvailPhys;
    unsigned long long ullTotalPageFile;
    unsigned long long ullAvailPageFile;
    unsigned long long ullTotalVirtual;
    unsigned long long ullAvailVirtual;
    unsigned long long ullAvailExtendedVirtual;
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX;

typedef union _LARGE_INTEGER {
    struct { DWORD LowPart; long HighPart; }; // GNU anonymous-member extension
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
typedef union _ULARGE_INTEGER {
    struct { DWORD LowPart; DWORD HighPart; }; // GNU anonymous-member extension
    ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

typedef struct _TIME_ZONE_INFORMATION {
    LONG Bias;
    CHAR StandardName[64];
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    CHAR DaylightName[64];
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
} TIME_ZONE_INFORMATION, *PTIME_ZONE_INFORMATION, *LPTIME_ZONE_INFORMATION;
#define TIME_ZONE_ID_UNKNOWN 0
#define TIME_ZONE_ID_STANDARD 1
#define TIME_ZONE_ID_DAYLIGHT 2

typedef OSVERSIONINFO RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;
typedef LONG (*RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

typedef struct tWAVEFORMAT {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
} WAVEFORMAT, *PWAVEFORMAT, *LPWAVEFORMAT;
typedef struct tWAVEFORMATEX {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;

typedef struct tagPALETTEENTRY {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
} PALETTEENTRY, *LPPALETTEENTRY;
typedef struct tagLOGPALETTE {
    WORD palVersion;
    WORD palNumEntries;
    PALETTEENTRY palPalEntry[1];
} LOGPALETTE, *PLOGPALETTE;

typedef struct tagPOINTFLOAT {
    FLOAT x;
    FLOAT y;
} POINTFLOAT;

typedef struct tagGLYPHMETRICSFLOAT {
    FLOAT gmfBlackBoxX;
    FLOAT gmfBlackBoxY;
    POINTFLOAT gmfptGlyphOrigin;
    FLOAT gmfCellIncX;
    FLOAT gmfCellIncY;
} GLYPHMETRICSFLOAT, *LPGLYPHMETRICSFLOAT;

typedef struct _RGNDATAHEADER {
    DWORD dwSize;
    DWORD iType;
    DWORD nCount;
    DWORD nRgnSize;
    RECT rcBound;
} RGNDATAHEADER, *PRGNDATAHEADER;

typedef struct _RGNDATA {
    RGNDATAHEADER rdh;
    char Buffer[1];
} RGNDATA, *NPRGNDATA, *LPRGNDATA;

typedef struct tagLOGFONTA {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    CHAR lfFaceName[32];
} LOGFONTA, *LPLOGFONTA;
#define LOGFONT LOGFONTA

typedef struct tagTEXTMETRICA {
    LONG tmHeight; LONG tmAscent; LONG tmDescent; LONG tmInternalLeading; LONG tmExternalLeading;
    LONG tmAveCharWidth; LONG tmMaxCharWidth; LONG tmWeight; LONG tmOverhang;
    LONG tmDigitizedAspectX; LONG tmDigitizedAspectY;
    BYTE tmFirstChar; BYTE tmLastChar; BYTE tmDefaultChar; BYTE tmBreakChar;
    BYTE tmItalic; BYTE tmUnderlined; BYTE tmStruckOut; BYTE tmPitchAndFamily; BYTE tmCharSet;
} TEXTMETRICA, *LPTEXTMETRICA;
#define TEXTMETRIC TEXTMETRICA
#define LPTEXTMETRIC LPTEXTMETRICA
typedef DWORD COLORREF, *LPCOLORREF;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize; LONG biWidth; LONG biHeight; WORD biPlanes; WORD biBitCount;
    DWORD biCompression; DWORD biSizeImage; LONG biXPelsPerMeter; LONG biYPelsPerMeter;
    DWORD biClrUsed; DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER, *LPBITMAPINFOHEADER;
typedef struct tagRGBQUAD {
    BYTE rgbBlue; BYTE rgbGreen; BYTE rgbRed; BYTE rgbReserved;
} RGBQUAD, *LPRGBQUAD;
typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[1];
} BITMAPINFO, *PBITMAPINFO, *LPBITMAPINFO;
typedef struct tagBITMAPFILEHEADER {
    WORD bfType; DWORD bfSize; WORD bfReserved1; WORD bfReserved2; DWORD bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER;

typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    unsigned long long ftCreationTime;
    unsigned long long ftLastAccessTime;
    unsigned long long ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD reserved0;
    DWORD reserved1;
    char cFileName[MAX_PATH];
    char cAlternateFileName[14];
} WIN32_FIND_DATA, *LPWIN32_FIND_DATA;

// ---------------------------------------------------------------------------
// Critical section → pthread recursive mutex
// ---------------------------------------------------------------------------
#include <pthread.h>

typedef struct _CRITICAL_SECTION {
    pthread_mutex_t mutex;
    unsigned int LockCount;
    unsigned long OwningThread;
} CRITICAL_SECTION, *LPCRITICAL_SECTION, RTL_CRITICAL_SECTION;

void InitializeCriticalSection(CRITICAL_SECTION* cs);
void InitializeCriticalSectionAndSpinCount(CRITICAL_SECTION* cs, unsigned long spin);
void EnterCriticalSection(CRITICAL_SECTION* cs);
void LeaveCriticalSection(CRITICAL_SECTION* cs);
void DeleteCriticalSection(CRITICAL_SECTION* cs);
BOOL TryEnterCriticalSection(CRITICAL_SECTION* cs);

// ---------------------------------------------------------------------------
// Threads / events / mutexes → pthread
// ---------------------------------------------------------------------------
typedef unsigned long (*LPTHREAD_START_ROUTINE)(void* lpThreadParameter);

HANDLE CreateThread(void* security, size_t stack, LPTHREAD_START_ROUTINE start,
                    void* param, DWORD flags, DWORD* tid);
HANDLE GetCurrentThread(void);
int GetCurrentThreadId(void);
DWORD GetCurrentProcessId(void);
BOOL CloseHandle(HANDLE h);
void ExitThread(DWORD code);
BOOL SetThreadPriority(HANDLE h, int prio);
BOOL TerminateThread(HANDLE h, DWORD exitCode);
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_HIGHEST 2
#define THREAD_PRIORITY_LOWEST -2
#define THREAD_PRIORITY_BELOW_NORMAL -1
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#define THREAD_PRIORITY_TIME_CRITICAL 15

HANDLE CreateEventA(void* attr, BOOL manual, BOOL initial, const char* name);
HANDLE CreateEventW(void* attr, BOOL manual, BOOL initial, const wchar_t* name);
#define CreateEvent CreateEventA
BOOL SetEvent(HANDLE h);
BOOL ResetEvent(HANDLE h);
DWORD WaitForSingleObject(HANDLE h, DWORD ms);
DWORD WaitForMultipleObjects(DWORD count, const HANDLE* handles, BOOL waitAll, DWORD ms);

HANDLE CreateSemaphoreA(void* attr, LONG initial, LONG max, const char* name);
BOOL ReleaseSemaphore(HANDLE h, LONG count, LONG* prev);

HANDLE CreateMutexA(void* attr, BOOL initialOwner, const char* name);
#define CreateMutex CreateMutexA
HANDLE OpenMutexA(DWORD access, BOOL inherit, const char* name);
#define OpenMutex OpenMutexA
BOOL ReleaseMutex(HANDLE h);

// ---------------------------------------------------------------------------
// Timing — interlocked with Dependencies/Utility/Utility/{time,thread}_compat.h
// ---------------------------------------------------------------------------
#ifndef GENERALS_PORT_COMPAT_TAKEN
#define GENERALS_PORT_COMPAT_TAKEN
#define TIMERR_NOERROR 0
unsigned int GetTickCount(void);
ULONGLONG GetTickCount64(void);
unsigned int timeGetTime(void);
int GetCurrentThreadId(void);
MMRESULT timeBeginPeriod(int);
MMRESULT timeEndPeriod(int);
void GetLocalTime(SYSTEMTIME* st);
void GetSystemTime(SYSTEMTIME* st);
void Sleep(int ms);
#endif // GENERALS_PORT_COMPAT_TAKEN
BOOL QueryPerformanceCounter(LONGLONG* counter);
BOOL QueryPerformanceFrequency(LONGLONG* frequency);

// ---------------------------------------------------------------------------
// Debug output / message boxes (log redirection)
// ---------------------------------------------------------------------------
void OutputDebugStringA(const char* s);
void OutputDebugStringW(const wchar_t* s);
#define OutputDebugString OutputDebugStringA
int MessageBoxA(void* hwnd, const char* text, const char* caption, unsigned type);
int MessageBoxW(void* hwnd, const wchar_t* text, const wchar_t* caption, unsigned type);
#define MessageBox MessageBoxA

// ---------------------------------------------------------------------------
// Interlocked
// ---------------------------------------------------------------------------
long InterlockedIncrement(volatile long* v);
long InterlockedDecrement(volatile long* v);
long InterlockedExchange(volatile long* v, long value);
long InterlockedExchangeAdd(volatile long* v, long value);
long InterlockedCompareExchange(volatile long* dest, long exchange, long comparand);

// ---------------------------------------------------------------------------
// System / module / directory / file helpers
// ---------------------------------------------------------------------------
DWORD GetModuleFileNameA(HMODULE mod, char* out, DWORD size);
#define GetModuleFileName GetModuleFileNameA
HMODULE GetModuleHandleA(const char* name);
#define GetModuleHandle GetModuleHandleA
void* GetProcAddress(HMODULE mod, const char* name);
HMODULE LoadLibraryA(const char* name);
#define LoadLibrary LoadLibraryA
HMODULE LoadLibraryExA(const char* name, HANDLE file, DWORD flags);
#define LoadLibraryEx LoadLibraryExA
#define DONT_RESOLVE_DLL_REFERENCES 1
#define LOAD_LIBRARY_AS_DATAFILE 2
BOOL FreeLibrary(HMODULE mod);
DWORD GetCurrentDirectoryA(DWORD size, char* out);
BOOL SetCurrentDirectoryA(const char* path);
BOOL CreateDirectoryA(const char* path, void* attr);
BOOL RemoveDirectoryA(const char* path);
BOOL DeleteFileA(const char* path);
BOOL CopyFileA(const char* src, const char* dst, BOOL failIfExists);
BOOL MoveFileA(const char* src, const char* dst);
DWORD GetFileAttributesA(const char* path);
BOOL GetDiskFreeSpaceA(const char* path, DWORD* spc, DWORD* bps, DWORD* freeClusters, DWORD* totalClusters);
#define GetCurrentDirectory GetCurrentDirectoryA
#define CreateDirectory CreateDirectoryA
#define DeleteFile DeleteFileA
#define CopyFile CopyFileA
#define MoveFile MoveFileA
#define GetFileAttributes GetFileAttributesA

const char* GetCommandLineA(void);
#define GetCommandLine GetCommandLineA
DWORD GetVersion(void);
BOOL GetVersionExA(OSVERSIONINFO* info);
#define GetVersionEx GetVersionExA
void GetSystemInfo(SYSTEM_INFO* info);
BOOL GlobalMemoryStatusEx(MEMORYSTATUSEX* info);
UINT GetWindowsDirectoryA(char* out, UINT size);
#define GetWindowsDirectory GetWindowsDirectoryA
UINT GetSystemDirectoryA(char* out, UINT size);
#define GetSystemDirectory GetSystemDirectoryA
DWORD GetEnvironmentVariableA(const char* name, char* buf, DWORD size);
#define GetEnvironmentVariable GetEnvironmentVariableA

// ---------------------------------------------------------------------------
// Registry (INI-backed on Android)
// ---------------------------------------------------------------------------
LONG RegOpenKeyExA(HKEY key, const char* sub, DWORD options, REGSAM sam, HKEY* result);
#define RegOpenKeyEx RegOpenKeyExA
LONG RegOpenKeyA(HKEY key, const char* sub, HKEY* result);
#define RegOpenKey RegOpenKeyA
LONG RegCreateKeyExA(HKEY key, const char* sub, DWORD reserved, char* cls,
                     DWORD options, REGSAM sam, void* secAttr, HKEY* result, DWORD* disposition);
#define RegCreateKeyEx RegCreateKeyExA
LONG RegQueryValueExA(HKEY key, const char* name, DWORD* reserved, DWORD* type,
                      LPBYTE data, DWORD* size);
#define RegQueryValueEx RegQueryValueExA
LONG RegSetValueExA(HKEY key, const char* name, DWORD reserved, DWORD type,
                    const BYTE* data, DWORD size);
#define RegSetValueEx RegSetValueExA
LONG RegCloseKey(HKEY key);
LONG RegDeleteValueA(HKEY key, const char* name);
LONG RegDeleteKeyA(HKEY key, const char* name);
#define RegDeleteValue RegDeleteValueA
#define RegDeleteKey RegDeleteKeyA
LONG RegQueryInfoKeyA(HKEY key, char* cls, DWORD* clsSize, DWORD* reserved,
                      DWORD* subKeys, DWORD* maxSubKeyLen, DWORD* maxClassLen,
                      DWORD* values, DWORD* maxValueNameLen, DWORD* maxValueLen,
                      DWORD* secDesc, void* lastWriteTime);
#define RegQueryInfoKey RegQueryInfoKeyA
LONG RegEnumKeyExA(HKEY key, DWORD index, char* name, DWORD* nameSize,
                   DWORD* reserved, char* cls, DWORD* clsSize, void* lastWriteTime);
#define RegEnumKeyEx RegEnumKeyExA
LONG RegEnumValueA(HKEY key, DWORD index, char* name, DWORD* nameSize,
                   DWORD* reserved, DWORD* type, LPBYTE data, DWORD* size);
#define RegEnumValue RegEnumValueA

// ---------------------------------------------------------------------------
// Find-file / file I/O
// ---------------------------------------------------------------------------
HANDLE FindFirstFileA(const char* pattern, WIN32_FIND_DATA* data);
#define FindFirstFile FindFirstFileA
BOOL FindNextFileA(HANDLE h, WIN32_FIND_DATA* data);
#define FindNextFile FindNextFileA
BOOL FindClose(HANDLE h);

HANDLE CreateFileA(const char* name, DWORD access, DWORD share, void* sec,
                   DWORD disposition, DWORD flags, HANDLE tmpl);
#define CreateFile CreateFileA
BOOL ReadFile(HANDLE h, void* buf, DWORD toRead, DWORD* read, void* overlapped);
BOOL WriteFile(HANDLE h, const void* buf, DWORD toWrite, DWORD* written, void* overlapped);
DWORD SetFilePointer(HANDLE h, LONG dist, LONG* distHigh, DWORD method);
DWORD GetFileSize(HANDLE h, DWORD* sizeHigh);
BOOL SetEndOfFile(HANDLE h);
BOOL FlushFileBuffers(HANDLE h);

// ---------------------------------------------------------------------------
// Process
// ---------------------------------------------------------------------------
void ExitProcess(UINT code);
#define GetCurrentProcess() ((HANDLE)(long long)-1)
BOOL TerminateProcess(HANDLE h, UINT code);

// ---------------------------------------------------------------------------
// GDI (stubs fail fast — the Android text path is GLES + Vazirmatn)
// ---------------------------------------------------------------------------
HGDIOBJ SelectObject(HDC hdc, HGDIOBJ obj);
BOOL DeleteObject(HGDIOBJ obj);
HDC CreateCompatibleDC(HDC hdc);
BOOL DeleteDC(HDC hdc);
HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO* bmi, UINT usage, void** bits, HANDLE section, DWORD offset);
BOOL BitBlt(HDC dst, int x, int y, int w, int h, HDC src, int sx, int sy, DWORD rop);
UINT SetDIBColorTable(HDC hdc, UINT start, UINT entries, const RGBQUAD* colors);
int GetDeviceCaps(HDC hdc, int index);
#define DIB_RGB_COLORS 0
#define BI_RGB 0
#define SRCCOPY 0x00CC0020
#define BITSPIXEL 12
#define PLANES 14

HFONT CreateFontA(int h, int w, int esc, int orient, int weight, DWORD italic, DWORD underline,
                  DWORD strikeout, DWORD charset, DWORD outprec, DWORD clipprec, DWORD quality,
                  DWORD pitchfamily, const char* face);
#define CreateFont CreateFontA
BOOL GetTextMetricsA(HDC hdc, TEXTMETRICA* tm);
#define GetTextMetrics GetTextMetricsA
HDC GetDC(HWND hwnd);
int ReleaseDC(HWND hwnd, HDC hdc);
BOOL GetTextExtentPoint32A(HDC hdc, const char* text, int len, SIZE* size);
#define GetTextExtentPoint32 GetTextExtentPoint32A
// Window/monitor management (DX8 wrapper window paths; Android owns the
// window via the Kotlin side — these stubs keep the wrapper linkable)
BOOL SetWindowPos(HWND hwnd, HWND after, int x, int y, int w, int h, UINT flags);
#define SWP_NOZORDER 0x0004
#define SWP_NOMOVE 0x0002
#define SWP_NOSIZE 0x0001
#define HWND_TOPMOST ((HWND)-1)
#define HWND_NOTOPMOST ((HWND)-2)
#define HWND_TOP ((HWND)0)
HWND GetDesktopWindow(void);
HMONITOR MonitorFromWindow(HWND hwnd, DWORD flags);
#define MONITOR_DEFAULTTOPRIMARY 1
#define MONITOR_DEFAULTTONEAREST 2
typedef struct tagMONITORINFO {
    DWORD cbSize;
    RECT rcMonitor;
    RECT rcWork;
    DWORD dwFlags;
} MONITORINFO, *LPMONITORINFO;
BOOL GetMonitorInfoA(HMONITOR mon, MONITORINFO* info);
#define GetMonitorInfo GetMonitorInfoA
#define MONITORINFOF_PRIMARY 1
BOOL SetDeviceGammaRamp(HDC hdc, void* ramp);
int GetSystemMetrics(int index);
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
LONG GetWindowLongA(HWND hwnd, int index);
#define GetWindowLong GetWindowLongA
#define GWL_STYLE (-16)
#define GWL_EXSTYLE (-20)
BOOL AdjustWindowRect(RECT* rect, DWORD style, BOOL menu);
BOOL GetClientRect(HWND hwnd, RECT* rect);
void SetRect(RECT* rect, int l, int t, int r, int b);
#define OF_WRITE 1
#define OF_CREATE 0x1000
#define OF_READ 0
LPVOID GlobalFreePtr(LPVOID mem); // GlobalFree alias used by WW3D2
#define GlobalFree GlobalFreePtr
#define GMEM_MOVEABLE 0x0002
#define GMEM_ZEROINIT 0x0040
HGLOBAL GlobalAllocA(UINT flags, SIZE_T bytes);
#define GlobalAlloc GlobalAllocA
#define GlobalAllocPtr GlobalAllocA
LPVOID GlobalLockA(HGLOBAL mem);
#define GlobalLock GlobalLockA
BOOL GlobalUnlockA(HGLOBAL mem);
#define GlobalUnlock GlobalUnlockA
int MulDiv(int n, int num, int den);
BOOL SetTextColor(HDC hdc, COLORREF color);
BOOL SetBkColor(HDC hdc, COLORREF color);
BOOL ExtTextOutW(HDC hdc, int x, int y, UINT options, const RECT* rect, const wchar_t* text, UINT len, const int* dx);
BOOL GetTextExtentPoint32W(HDC hdc, const wchar_t* text, int len, SIZE* size);
#define LOWORD(l) ((WORD)(((DWORD)(l)) & 0xFFFF))
#define HIWORD(l) ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))
#define LOBYTE(w) ((BYTE)(((DWORD)(w)) & 0xFF))
#define HIBYTE(w) ((BYTE)((((DWORD)(w)) >> 8) & 0xFF))
BOOL GetWindowRect(HWND hwnd, RECT* rect);
#define ETO_OPAQUE 0x0002
#define ETO_CLIPPED 0x0004
#define RGB(r, g, b) ((COLORREF)(((BYTE)(r)) | ((WORD)((BYTE)(g)) << 8) | ((DWORD)((BYTE)(b)) << 16)))
#define VARIABLE_PITCH 0xFF
#define DEFAULT_PITCH 0
#define FIXED_PITCH 1
#define OUT_DEFAULT_PRECIS 0
#define OUT_TT_PRECIS 4
#define CLIP_DEFAULT_PRECIS 0
#define ANTIALIASED_QUALITY 4
#define DEFAULT_QUALITY 0
#define CLEARTYPE_QUALITY 5
#define DEFAULT_CHARSET 1
#define FW_NORMAL 400
#define FW_BOLD 700

// ---------------------------------------------------------------------------
// lstr family (WW3D2 string helpers)
// ---------------------------------------------------------------------------
LPSTR lstrcpyA(LPSTR dst, const char* src);
LPSTR lstrcpynA(LPSTR dst, const char* src, int len);
LPSTR lstrcatA(LPSTR dst, const char* src);
int lstrlenA(const char* s);
int lstrcmpiA(const char* a, const char* b);
#define lstrcpy lstrcpyA
#define lstrcpyn lstrcpynA
#define lstrcat lstrcatA
#define lstrlen lstrlenA
#define lstrcmpi lstrcmpiA
char* strupr_shim(char* s);
char* strlwr_shim(char* s);
#ifndef strupr
#define strupr strupr_shim
#endif
#ifndef strlwr
#define strlwr strlwr_shim
#endif

// ---------------------------------------------------------------------------
// Error composition + last error
// ---------------------------------------------------------------------------
#define FACILITY_ITF 4
#define FACILITY_WIN32 7
#define SEVERITY_SUCCESS 0
#define SEVERITY_ERROR 3
#define MAKE_HRESULT(sev, fac, code) ((HRESULT)(((unsigned long)(sev)<<31)|((unsigned long)(fac)<<16)|(unsigned long)(code)))
DWORD GetTimeZoneInformation(TIME_ZONE_INFORMATION* tzi);
LONG RtlGetVersion(void* info); // PRTL_OSVERSIONINFOW; version bytes match GetVersionEx
HMODULE LoadLibraryExA(const char* name, HANDLE file, DWORD flags);
#define LoadLibraryEx LoadLibraryExA
#define DONT_RESOLVE_DLL_REFERENCES 1
#define LOAD_LIBRARY_AS_DATAFILE 2
DWORD FormatMessageA(DWORD flags, const void* source, DWORD msgId, DWORD langId,
                     char* buf, DWORD size, void* args);
#define FormatMessage FormatMessageA
DWORD FormatMessageW(DWORD flags, const void* source, DWORD msgId, DWORD langId,
                     wchar_t* buf, DWORD size, void* args);
char* itoa(int value, char* str, int base);
#define _itoa itoa
#define _ultoa utoa
#define LOCALE_USER_DEFAULT 0x0400
#define VARCMP_LT 0
#define VARCMP_EQ 1
#define VARCMP_GT 2
#define VARCMP_NULL 3
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x00000200
#define LANG_NEUTRAL 0

// Rotation intrinsics (CRC hot paths)
static inline unsigned long _lrotl(unsigned long v, int s) {
    s &= 31;
    return s ? ((v << s) | (v >> (32 - s))) : v;
}
static inline unsigned long _lrotr(unsigned long v, int s) {
    s &= 31;
    return s ? ((v >> s) | (v << (32 - s))) : v;
}

int GetLastError(void); // lzhl's CompLib declares int; DWORD callers still work
#define SetLastError(x) ((void)(x))

// Timing helpers referencing the structs above
BOOL QueryPerformanceCounter(LONGLONG* counter);
BOOL QueryPerformanceFrequency(LONGLONG* frequency);
