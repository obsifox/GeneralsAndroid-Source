// win32shim.cpp — POSIX implementations behind the windows.h shim.
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
// Master Prompt §35: every workaround documented at the site of the hack.

#include "windows.h"
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysconf.h>
#include <semaphore.h>
#include <fnmatch.h>
#include <string>
#include <vector>
#include "android_log_shim.h"

#define SHIM_LOG_TAG "win32shim"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, SHIM_LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, SHIM_LOG_TAG, __VA_ARGS__)

// ---------------------------------------------------------------------------
// Critical sections
// ---------------------------------------------------------------------------
extern "C" {

void InitializeCriticalSection(CRITICAL_SECTION* cs) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); // Win32 CS is recursive
    pthread_mutex_init(&cs->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    cs->LockCount = 0;
    cs->OwningThread = 0;
}

void InitializeCriticalSectionAndSpinCount(CRITICAL_SECTION* cs, unsigned long) {
    InitializeCriticalSection(cs);
}

void EnterCriticalSection(CRITICAL_SECTION* cs) { pthread_mutex_lock(&cs->mutex); }
void LeaveCriticalSection(CRITICAL_SECTION* cs) { pthread_mutex_unlock(&cs->mutex); }
void DeleteCriticalSection(CRITICAL_SECTION* cs) { pthread_mutex_destroy(&cs->mutex); }
BOOL TryEnterCriticalSection(CRITICAL_SECTION* cs) {
    return pthread_mutex_trylock(&cs->mutex) == 0 ? TRUE : FALSE;
}

// ---------------------------------------------------------------------------
// Threads
// ---------------------------------------------------------------------------
struct Win32ShimThread_ {
    pthread_t thread;
    LPTHREAD_START_ROUTINE start;
    void* param;
    DWORD tid;
};

static void* shim_thread_trampoline(void* arg) {
    Win32ShimThread_* t = static_cast<Win32ShimThread_*>(arg);
    unsigned long rc = t->start(t->param);
    return reinterpret_cast<void*>(rc);
}

HANDLE CreateThread(void*, size_t, LPTHREAD_START_ROUTINE start, void* param,
                    DWORD, DWORD* tid) {
    Win32ShimThread_* t = new Win32ShimThread_();
    t->start = start;
    t->param = param;
    t->tid = 0;
    if (pthread_create(&t->thread, nullptr, shim_thread_trampoline, t) != 0) {
        delete t;
        return nullptr;
    }
    if (tid) *tid = static_cast<DWORD>(t->tid); // real id set by caller context if needed
    return t;
}

HANDLE GetCurrentThread(void) { return (HANDLE)(long long)-2; }
int GetCurrentThreadId(void) { return static_cast<int>(gettid()); }
DWORD GetCurrentProcessId(void) { return static_cast<DWORD>(getpid()); }

BOOL CloseHandle(HANDLE h) {
    if (!h || h == (HANDLE)(long long)-1 || h == (HANDLE)(long long)-2) return TRUE;
    delete static_cast<Win32ShimThread_*>(h);
    return TRUE;
}

uintptr_t _beginthread(void (*start)(void*), void* /*stack*/, void* arg) {
    // §35 workaround: MSVC _beginthread returns the OS handle; WWLib only
    // uses it as an opaque token for SetThreadPriority/WaitForSingleObject.
    struct CtxHeap { void (*fn)(void*); void* arg; };
    CtxHeap* ctx = new CtxHeap{start, arg};
    Win32ShimThread_* t = new Win32ShimThread_();
    t->start = [](void* p) -> unsigned long {
        CtxHeap* c = static_cast<CtxHeap*>(p);
        c->fn(c->arg);
        delete c;
        return 0;
    };
    t->param = ctx;
    if (pthread_create(&t->thread, nullptr, shim_thread_trampoline, t) != 0) {
        delete t;
        delete ctx;
        return 0;
    }
    return reinterpret_cast<uintptr_t>(t);
}

void ExitThread(DWORD) { pthread_exit(nullptr); }

BOOL SetThreadPriority(HANDLE, int) { return TRUE; } // best-effort on Android

// ---------------------------------------------------------------------------
// Events (pthread cond + state)
// ---------------------------------------------------------------------------
struct Win32ShimEvent {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    BOOL manual;
    BOOL signaled;
};

HANDLE CreateEventA(void*, BOOL manual, BOOL initial, const char*) {
    Win32ShimEvent* e = new Win32ShimEvent();
    pthread_mutex_init(&e->mutex, nullptr);
    pthread_cond_init(&e->cond, nullptr);
    e->manual = manual;
    e->signaled = initial;
    return e;
}

HANDLE CreateEventW(void* attr, BOOL manual, BOOL initial, const wchar_t*) {
    return CreateEventA(attr, manual, initial, nullptr);
}

BOOL SetEvent(HANDLE h) {
    Win32ShimEvent* e = static_cast<Win32ShimEvent*>(h);
    if (!e) return FALSE;
    pthread_mutex_lock(&e->mutex);
    e->signaled = TRUE;
    if (e->manual) pthread_cond_broadcast(&e->cond);
    else pthread_cond_signal(&e->cond);
    pthread_mutex_unlock(&e->mutex);
    return TRUE;
}

BOOL ResetEvent(HANDLE h) {
    Win32ShimEvent* e = static_cast<Win32ShimEvent*>(h);
    if (!e) return FALSE;
    pthread_mutex_lock(&e->mutex);
    e->signaled = FALSE;
    pthread_mutex_unlock(&e->mutex);
    return TRUE;
}

DWORD WaitForSingleObject(HANDLE h, DWORD ms) {
    Win32ShimEvent* e = static_cast<Win32ShimEvent*>(h);
    if (!e) return WAIT_FAILED;
    pthread_mutex_lock(&e->mutex);
    while (!e->signaled) {
        if (ms == INFINITE) {
            pthread_cond_wait(&e->cond, &e->mutex);
        } else {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += ms / 1000;
            ts.tv_nsec += (ms % 1000) * 1000000L;
            if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
            int rc = pthread_cond_timedwait(&e->cond, &e->mutex, &ts);
            if (rc == ETIMEDOUT) { pthread_mutex_unlock(&e->mutex); return WAIT_TIMEOUT; }
        }
    }
    if (!e->manual) e->signaled = FALSE;
    pthread_mutex_unlock(&e->mutex);
    return WAIT_OBJECT_0;
}

DWORD WaitForMultipleObjects(DWORD, const HANDLE*, BOOL, DWORD) {
    // Not used by the non-networking engine paths; extended later with real plumbing.
    LOGW("WaitForMultipleObjects: stubbed, returns WAIT_OBJECT_0");
    return WAIT_OBJECT_0;
}

// Mutexes
struct Win32ShimMutex {
    pthread_mutex_t mutex;
    BOOL owned;
};
HANDLE CreateMutexA(void*, BOOL initialOwner, const char*) {
    Win32ShimMutex* m = new Win32ShimMutex();
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    m->owned = FALSE;
    if (initialOwner) { pthread_mutex_lock(&m->mutex); m->owned = TRUE; }
    return (HANDLE)m;
}
HANDLE OpenMutexA(DWORD, BOOL, const char*) { return nullptr; }
BOOL ReleaseMutex(HANDLE h) {
    Win32ShimMutex* m = static_cast<Win32ShimMutex*>(h);
    if (!m) return FALSE;
    if (m->owned) { pthread_mutex_unlock(&m->mutex); m->owned = FALSE; }
    return TRUE;
}
BOOL TerminateThread(HANDLE, DWORD) {
    LOGW("TerminateThread: not supported on POSIX — ignored");
    return TRUE;
}

// Semaphores
HANDLE CreateSemaphoreA(void*, LONG initial, LONG, const char*) {
    sem_t* s = new sem_t();
    sem_init(s, 0, initial > 0 ? initial : 0);
    return (HANDLE)s;
}

BOOL ReleaseSemaphore(HANDLE h, LONG count, LONG* prev) {
    sem_t* s = static_cast<sem_t*>(h);
    if (!s) return FALSE;
    if (prev) *prev = 0;
    for (LONG i = 0; i < count; i++) sem_post(s);
    return TRUE;
}

// ---------------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------------
static struct timespec shim_boot_time = {0, 0};
static bool shim_boot_initialized = false;

static LONGLONG shim_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (LONGLONG)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

DWORD GetTickCount(void) { return static_cast<DWORD>(shim_now_ms()); }
ULONGLONG GetTickCount64(void) { return static_cast<ULONGLONG>(shim_now_ms()); }
unsigned int timeGetTime(void) { return static_cast<unsigned int>(shim_now_ms()); }
MMRESULT timeBeginPeriod(int) { return TIMERR_NOERROR; }
MMRESULT timeEndPeriod(int) { return TIMERR_NOERROR; }

BOOL QueryPerformanceCounter(LONGLONG* counter) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    *counter = (LONGLONG)ts.tv_sec * 1000000000LL + ts.tv_nsec;
    return TRUE;
}

BOOL QueryPerformanceFrequency(LONGLONG* frequency) {
    *frequency = 1000000000LL; // ns base
    return TRUE;
}

void GetLocalTime(SYSTEMTIME* st) {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    memset(st, 0, sizeof(*st));
    st->wYear = t.tm_year + 1900;
    st->wMonth = t.tm_mon + 1;
    st->wDay = t.tm_mday;
    st->wDayOfWeek = t.tm_wday;
    st->wHour = t.tm_hour;
    st->wMinute = t.tm_min;
    st->wSecond = t.tm_sec;
    st->wMilliseconds = 0;
}

void GetSystemTime(SYSTEMTIME* st) {
    time_t now = time(nullptr);
    struct tm t;
    gmtime_r(&now, &t);
    memset(st, 0, sizeof(*st));
    st->wYear = t.tm_year + 1900;
    st->wMonth = t.tm_mon + 1;
    st->wDay = t.tm_mday;
    st->wDayOfWeek = t.tm_wday;
    st->wHour = t.tm_hour;
    st->wMinute = t.tm_min;
    st->wSecond = t.tm_sec;
    st->wMilliseconds = 0;
}

void Sleep(int ms) {
    if (ms < 0) ms = 0;
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, nullptr);
}

// ---------------------------------------------------------------------------
// Debug output / message boxes
// ---------------------------------------------------------------------------
void OutputDebugStringA(const char* s) { LOGD("%s", s ? s : "(null)"); }
void OutputDebugStringW(const wchar_t* s) { LOGD("%ls", s ? s : L"(null)"); }

DWORD FormatMessageW(DWORD, const void*, DWORD, DWORD, wchar_t* buf, DWORD, void*) {
    if (buf) buf[0] = 0;
    return 0;
}

char* itoa(int value, char* str, int base) {
    if (!str) return str;
    if (base == 10) { snprintf(str, 33, "%d", value); return str; }
    if (base == 16) { snprintf(str, 33, "%x", value); return str; }
    snprintf(str, 33, "%d", value);
    return str;
}

int GetLastError(void) { return errno; }

// —— GDI stubs (fail fast; Android text rendering is GLES-based) ——
HGDIOBJ SelectObject(HDC, HGDIOBJ obj) { return obj; }
BOOL DeleteObject(HGDIOBJ) { return TRUE; }
HDC CreateCompatibleDC(HDC) { return nullptr; }
BOOL DeleteDC(HDC) { return TRUE; }
HBITMAP CreateDIBSection(HDC, const BITMAPINFO*, UINT, void** bits, HANDLE, DWORD) {
    if (bits) *bits = nullptr;
    return nullptr; // renderer backend replaces this path
}
BOOL BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD) { return FALSE; }
UINT SetDIBColorTable(HDC, UINT, UINT, const RGBQUAD*) { return 0; }
int GetDeviceCaps(HDC, int index) { return index == BITSPIXEL ? 32 : 0; }
BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT) { return TRUE; }
HWND GetDesktopWindow(void) { return nullptr; }
HMONITOR MonitorFromWindow(HWND, DWORD) { return nullptr; }
BOOL GetMonitorInfoA(HMONITOR, MONITORINFO* info) {
    if (info) { memset(info, 0, sizeof(*info)); info->cbSize = sizeof(MONITORINFO);
        info->rcMonitor.right = 1920; info->rcMonitor.bottom = 1080; }
    return TRUE;
}
BOOL SetDeviceGammaRamp(HDC, void*) { return FALSE; }
int GetSystemMetrics(int index) { return index == SM_CXSCREEN ? 1920 : (index == SM_CYSCREEN ? 1080 : 0); }
LONG GetWindowLongA(HWND, int) { return 0; }
BOOL AdjustWindowRect(RECT* rect, DWORD, BOOL) {
    if (rect) { rect->left -= 0; rect->top -= 0; }
    return TRUE;
}
BOOL GetClientRect(HWND, RECT* rect) {
    if (rect) { rect->left = rect->top = 0; rect->right = 640; rect->bottom = 480; }
    return TRUE;
}
void SetRect(RECT* rect, int l, int t, int r, int b) {
    if (rect) { rect->left = l; rect->top = t; rect->right = r; rect->bottom = b; }
}
LPVOID GlobalFreePtr(LPVOID mem) { free(mem); return nullptr; }
HGLOBAL GlobalAllocA(UINT flags, SIZE_T bytes) {
    void* p = malloc(bytes);
    if (p && (flags & GMEM_ZEROINIT)) memset(p, 0, bytes);
    return (HGLOBAL)p;
}
LPVOID GlobalLockA(HGLOBAL mem) { return (LPVOID)mem; }
BOOL GlobalUnlockA(HGLOBAL) { return TRUE; }
int MulDiv(int n, int num, int den) {
    if (!den) return 0;
    return static_cast<int>((static_cast<long long>(n) * num) / den);
}
BOOL GetWindowRect(HWND, RECT* rect) {
    if (rect) { rect->left = rect->top = 0; rect->right = 640; rect->bottom = 480; }
    return TRUE;
}
HFONT CreateFontA(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, const char*) {
    return nullptr; // font backend is GLES + Vazirmatn (phase 11)
}
BOOL GetTextMetricsA(HDC, TEXTMETRICA* tm) {
    if (tm) memset(tm, 0, sizeof(*tm));
    return TRUE;
}
HDC GetDC(HWND) { return nullptr; }
int ReleaseDC(HWND, HDC) { return 1; }
BOOL SetTextColor(HDC, COLORREF) { return TRUE; }
BOOL SetBkColor(HDC, COLORREF) { return TRUE; }
BOOL ExtTextOutW(HDC, int, int, UINT, const RECT*, const wchar_t*, UINT, const int*) { return TRUE; }
BOOL GetTextExtentPoint32W(HDC, const wchar_t*, int, SIZE* size) {
    if (size) { size->cx = 0; size->cy = 0; }
    return TRUE;
}
BOOL GetTextExtentPoint32A(HDC, const char*, int, SIZE* size) {
    if (size) { size->cx = 0; size->cy = 0; }
    return TRUE;
}

LPSTR lstrcpyA(LPSTR dst, const char* src) {
    if (!dst || !src) return dst;
    strcpy(dst, src);
    return dst;
}
LPSTR lstrcpynA(LPSTR dst, const char* src, int len) {
    if (!dst || !src || len <= 0) return dst;
    strncpy(dst, src, len - 1);
    dst[len - 1] = 0;
    return dst;
}
LPSTR lstrcatA(LPSTR dst, const char* src) {
    if (!dst || !src) return dst;
    strcat(dst, src);
    return dst;
}
int lstrlenA(const char* s) { return s ? static_cast<int>(strlen(s)) : 0; }
int lstrcmpiA(const char* a, const char* b) {
    if (!a || !b) return -1;
    return strcasecmp(a, b);
}
char* strupr_shim(char* s) {
    if (s) for (char* p = s; *p; ++p) *p = toupper(static_cast<unsigned char>(*p));
    return s;
}
char* strlwr_shim(char* s) {
    if (s) for (char* p = s; *p; ++p) *p = tolower(static_cast<unsigned char>(*p));
    return s;
}

DWORD GetTimeZoneInformation(TIME_ZONE_INFORMATION* tzi) {
    if (tzi) { memset(tzi, 0, sizeof(*tzi)); tzi->Bias = 0; }
    return TIME_ZONE_ID_UNKNOWN;
}

LONG RtlGetVersion(void* info) {
    // §35 workaround: ntdll RtlGetVersion is a Windows API; the port reports
    // a static modern-Windows identity so engine checks take the safe path.
    OSVERSIONINFO* v = static_cast<OSVERSIONINFO*>(info);
    if (v) {
        v->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        v->dwMajorVersion = 5;
        v->dwMinorVersion = 1;
        v->dwBuildNumber = 2600;
        v->dwPlatformId = 2;
        strncpy(v->szCSDVersion, "Android port", sizeof(v->szCSDVersion) - 1);
    }
    return 0;
}

HMODULE LoadLibraryExA(const char* name, HANDLE, DWORD) { return LoadLibraryA(name); }

int MessageBoxA(void*, const char* text, const char* caption, unsigned) {
    LOGW("MessageBox: [%s] %s", caption ? caption : "?", text ? text : "?");
    return IDOK;
}

int MessageBoxW(void*, const wchar_t* text, const wchar_t* caption, unsigned) {
    LOGW("MessageBoxW: called (text omitted)");
    return IDOK;
}

// ---------------------------------------------------------------------------
// Interlocked (GCC/Clang builtins)
// ---------------------------------------------------------------------------
long InterlockedIncrement(volatile long* v) { return __sync_add_and_fetch(v, 1); }
long InterlockedDecrement(volatile long* v) { return __sync_sub_and_fetch(v, 1); }
long InterlockedExchange(volatile long* v, long value) { return __sync_lock_test_and_set(v, value); }
long InterlockedExchangeAdd(volatile long* v, long value) { return __sync_fetch_and_add(v, value); }
long InterlockedCompareExchange(volatile long* dest, long exchange, long comparand) {
    return __sync_val_compare_and_swap(dest, comparand, exchange);
}

// ---------------------------------------------------------------------------
// System / module / directory helpers
// ---------------------------------------------------------------------------
DWORD GetModuleFileNameA(HMODULE, char* out, DWORD size) {
    // On Android there is no module path; engine paths come from JNI bootstrap.
    if (out && size) { strncpy(out, "/data/app/generals", size - 1); out[size - 1] = 0; }
    return static_cast<DWORD>(strlen(out));
}

HMODULE GetModuleHandleA(const char*) { return nullptr; }
void* GetProcAddress(HMODULE, const char*) { return nullptr; } // real dlopen/dlsym wiring where needed
HMODULE LoadLibraryA(const char* name) { LOGW("LoadLibrary(%s): not supported", name); return nullptr; }
BOOL FreeLibrary(HMODULE) { return TRUE; }

DWORD GetCurrentDirectoryA(DWORD size, char* out) {
    if (out && size) { strncpy(out, "/", size - 1); out[size - 1] = 0; }
    return 1;
}

BOOL SetCurrentDirectoryA(const char* path) { return chdir(path) == 0 ? TRUE : FALSE; }

BOOL CreateDirectoryA(const char* path, void*) {
    if (!path) return FALSE;
    if (mkdir(path, 0775) == 0) return TRUE;
    return errno == EEXIST ? TRUE : FALSE;
}

BOOL RemoveDirectoryA(const char* path) { return rmdir(path) == 0 ? TRUE : FALSE; }
BOOL DeleteFileA(const char* path) { return unlink(path) == 0 ? TRUE : FALSE; }

BOOL CopyFileA(const char* src, const char* dst, BOOL failIfExists) {
    if (!src || !dst) return FALSE;
    FILE* in = fopen(src, "rb");
    if (!in) return FALSE;
    if (failIfExists && fopen(dst, "rb")) {
        fclose(in);
        return FALSE;
    }
    FILE* out = fopen(dst, "wb");
    if (!out) { fclose(in); return FALSE; }
    char buf[65536];
    size_t n;
    BOOL ok = TRUE;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) { ok = FALSE; break; }
    }
    fclose(in);
    fclose(out);
    return ok;
}

BOOL MoveFileA(const char* src, const char* dst) { return rename(src, dst) == 0 ? TRUE : FALSE; }

DWORD GetFileAttributesA(const char* path) {
    if (!path) return 0xFFFFFFFF;
    struct stat st;
    if (stat(path, &st) != 0) return 0xFFFFFFFF;
    DWORD attr = FILE_ATTRIBUTE_NORMAL;
    if (S_ISDIR(st.st_mode)) attr |= FILE_ATTRIBUTE_DIRECTORY;
    if (!(st.st_mode & S_IWUSR)) attr |= FILE_ATTRIBUTE_READONLY;
    return attr;
}

BOOL GetDiskFreeSpaceA(const char*, DWORD* spc, DWORD* bps, DWORD* freeClusters, DWORD* totalClusters) {
    // Conservative defaults; real statvfs wiring lands with the storage phase.
    if (spc) *spc = 1;
    if (bps) *bps = 4096;
    if (freeClusters) *freeClusters = 1 << 20; // ~4 GB reported
    if (totalClusters) *totalClusters = 1 << 21;
    return TRUE;
}

static char g_cmdline[1024] = "";

const char* GetCommandLineA(void) { return g_cmdline; }
void shim_set_command_line(const char* cl) {
    if (cl) { strncpy(g_cmdline, cl, sizeof(g_cmdline) - 1); g_cmdline[sizeof(g_cmdline) - 1] = 0; }
}

DWORD GetVersion(void) { return 0x0000010A; } // pretend Windows XP-ish

BOOL GetVersionExA(OSVERSIONINFO* info) {
    if (!info) return FALSE;
    info->dwMajorVersion = 5;
    info->dwMinorVersion = 1;
    info->dwBuildNumber = 2600;
    info->dwPlatformId = 2;
    strncpy(info->szCSDVersion, "Android shim", sizeof(info->szCSDVersion) - 1);
    return TRUE;
}

void GetSystemInfo(SYSTEM_INFO* info) {
    memset(info, 0, sizeof(*info));
    info->dwPageSize = static_cast<DWORD>(sysconf(_SC_PAGESIZE));
    info->dwNumberOfProcessors = static_cast<DWORD>(sysconf(_SC_NPROCESSORS_ONLN));
    info->dwProcessorArchitecture = PROCESSOR_ARCHITECTURE_ARM64;
    info->dwAllocationGranularity = info->dwPageSize;
}

BOOL GlobalMemoryStatusEx(MEMORYSTATUSEX* info) {
    if (!info) return FALSE;
    info->dwLength = sizeof(*info);
    long pages = sysconf(_SC_PHYS_PAGES);
    long avail = sysconf(_SC_AVPHYS_PAGES);
    long psize = sysconf(_SC_PAGESIZE);
    info->ullTotalPhys = static_cast<unsigned long long>(pages) * psize;
    info->ullAvailPhys = static_cast<unsigned long long>(avail) * psize;
    info->ullTotalPageFile = info->ullTotalPhys;
    info->ullAvailPageFile = info->ullAvailPhys;
    info->ullTotalVirtual = info->ullTotalPhys;
    info->ullAvailVirtual = info->ullAvailPhys;
    info->dwMemoryLoad = info->ullTotalPhys
        ? static_cast<DWORD>(100 - 100 * info->ullAvailPhys / info->ullTotalPhys) : 0;
    return TRUE;
}

UINT GetWindowsDirectoryA(char* out, UINT size) {
    // Engine expects a "windows dir" for some defaults — map to app internal root.
    if (out && size) { strncpy(out, "/data", size - 1); out[size - 1] = 0; }
    return static_cast<UINT>(strlen(out));
}

UINT GetSystemDirectoryA(char* out, UINT size) { return GetWindowsDirectoryA(out, size); }

DWORD GetEnvironmentVariableA(const char* name, char* buf, DWORD size) {
    const char* v = getenv(name);
    if (!v) return 0;
    DWORD len = static_cast<DWORD>(strlen(v));
    if (buf && size) { strncpy(buf, v, size - 1); buf[size - 1] = 0; }
    return len;
}

// ---------------------------------------------------------------------------
// Registry → INI in app internal storage (options/serials survive updates)
// ---------------------------------------------------------------------------
static std::string shim_ini_path() {
    return "/data/data/com.obsifox.generals/files/shim_registry.ini";
}

static void shim_ini_write(const std::string& key, const std::string& value) {
    FILE* f = fopen(shim_ini_path().c_str(), "a");
    if (!f) return;
    fprintf(f, "%s=%s\n", key.c_str(), value.c_str());
    fclose(f);
}

static bool shim_ini_read(const std::string& key, std::string& out) {
    FILE* f = fopen(shim_ini_path().c_str(), "r");
    if (!f) return false;
    char line[1024];
    bool found = false;
    while (fgets(line, sizeof(line), f)) {
        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        std::string k(line);
        std::string v(eq + 1);
        while (!v.empty() && (v.back() == '\n' || v.back() == '\r')) v.pop_back();
        if (k == key) { out = v; found = true; break; }
    }
    fclose(f);
    return found;
}

LONG RegOpenKeyExA(HKEY, const char*, DWORD, REGSAM, HKEY* result) {
    if (result) *result = (HKEY)0x1; // opaque non-null token
    return ERROR_SUCCESS;
}

LONG RegOpenKeyA(HKEY key, const char* sub, HKEY* result) {
    return RegOpenKeyExA(key, sub, 0, KEY_READ, result);
}

LONG RegCreateKeyExA(HKEY, const char*, DWORD, char*, DWORD, REGSAM, void*,
                     HKEY* result, DWORD* disposition) {
    if (result) *result = (HKEY)0x1;
    if (disposition) *disposition = 1;
    return ERROR_SUCCESS;
}

LONG RegQueryValueExA(HKEY key, const char* name, DWORD* reserved, DWORD* type,
                      LPBYTE data, DWORD* size) {
    if (!name) return ERROR_INVALID_PARAMETER;
    std::string v;
    std::string full = std::string("HKEY_") + name;
    if (!shim_ini_read(full, v)) return ERROR_FILE_NOT_FOUND;
    if (type) *type = REG_SZ;
    if (data && size) {
        DWORD need = static_cast<DWORD>(v.size() + 1);
        if (*size < need) { *size = need; return ERROR_MORE_DATA; }
        memcpy(data, v.c_str(), need);
        *size = need;
    }
    return ERROR_SUCCESS;
}

LONG RegSetValueExA(HKEY, const char* name, DWORD, DWORD, const BYTE* data, DWORD size) {
    if (!name || !data) return ERROR_INVALID_PARAMETER;
    std::string full = std::string("HKEY_") + name;
    shim_ini_write(full, std::string(reinterpret_cast<const char*>(data), strnlen(reinterpret_cast<const char*>(data), size)));
    return ERROR_SUCCESS;
}

LONG RegCloseKey(HKEY) { return ERROR_SUCCESS; }
LONG RegDeleteValueA(HKEY, const char*) { return ERROR_SUCCESS; }
LONG RegDeleteKeyA(HKEY, const char*) { return ERROR_SUCCESS; }
LONG RegQueryInfoKeyA(HKEY, char*, DWORD*, DWORD*, DWORD* subKeys, DWORD*, DWORD*,
                      DWORD* values, DWORD*, DWORD*, DWORD*, void*) {
    if (subKeys) *subKeys = 0;
    if (values) *values = 0;
    return ERROR_SUCCESS;
}
LONG RegEnumKeyExA(HKEY, DWORD, char*, DWORD*, DWORD*, char*, DWORD*, void*) { return ERROR_NO_MORE_ITEMS; }
LONG RegEnumValueA(HKEY, DWORD, char*, DWORD*, DWORD*, DWORD*, LPBYTE, DWORD*) { return ERROR_NO_MORE_ITEMS; }

// ---------------------------------------------------------------------------
// FindFirstFile / FindNextFile / FindClose
// ---------------------------------------------------------------------------
struct ShimFind {
    std::vector<std::string> entries;
    size_t index = 0;
    std::string base;
};

HANDLE FindFirstFileA(const char* pattern, WIN32_FIND_DATA* data) {
    if (!pattern || !data) return INVALID_HANDLE_VALUE;
    std::string p(pattern);
    size_t slash = p.find_last_of('/');
    std::string dir = (slash == std::string::npos) ? "." : p.substr(0, slash);
    std::string wc = (slash == std::string::npos) ? p : p.substr(slash + 1);
    if (dir.empty()) dir = "/";

    DIR* d = opendir(dir.c_str());
    if (!d) return INVALID_HANDLE_VALUE;

    // Translate simple wildcard (* and ?) to fnmatch
    ShimFind* f = new ShimFind();
    f->base = dir;
    struct dirent* de;
    while ((de = readdir(d)) != nullptr) {
        std::string name(de->d_name);
        if (name == "." || name == "..") continue;
        if (wc == "*" || wc == "*.*" || fnmatch(wc.c_str(), name.c_str(), 0) == 0) {
            f->entries.push_back(name);
        }
    }
    closedir(d);

    if (f->entries.empty()) { delete f; return INVALID_HANDLE_VALUE; }
    f->index = 0;
    // fill first entry
    std::string full = f->base + "/" + f->entries[0];
    struct stat st;
    memset(data, 0, sizeof(*data));
    strncpy(data->cFileName, f->entries[0].c_str(), MAX_PATH - 1);
    if (stat(full.c_str(), &st) == 0) {
        data->nFileSizeLow = static_cast<DWORD>(st.st_size);
        data->nFileSizeHigh = static_cast<DWORD>(st.st_size >> 32);
        if (S_ISDIR(st.st_mode)) data->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
        else data->dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;
    }
    return f;
}

BOOL FindNextFileA(HANDLE h, WIN32_FIND_DATA* data) {
    ShimFind* f = static_cast<ShimFind*>(h);
    if (!f || !data || f->index + 1 >= f->entries.size()) return FALSE;
    f->index++;
    std::string full = f->base + "/" + f->entries[f->index];
    struct stat st;
    memset(data, 0, sizeof(*data));
    strncpy(data->cFileName, f->entries[f->index].c_str(), MAX_PATH - 1);
    if (stat(full.c_str(), &st) == 0) {
        data->nFileSizeLow = static_cast<DWORD>(st.st_size);
        data->nFileSizeHigh = static_cast<DWORD>(st.st_size >> 32);
        if (S_ISDIR(st.st_mode)) data->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
        else data->dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;
    }
    return TRUE;
}

BOOL FindClose(HANDLE h) {
    ShimFind* f = static_cast<ShimFind*>(h);
    delete f;
    return TRUE;
}

// ---------------------------------------------------------------------------
// File I/O
// ---------------------------------------------------------------------------
HANDLE CreateFileA(const char* name, DWORD access, DWORD, void*,
                   DWORD disposition, DWORD, HANDLE) {
    if (!name) return INVALID_HANDLE_VALUE;
    const char* mode;
    if (disposition == CREATE_ALWAYS) mode = "wb+";
    else if (disposition == OPEN_ALWAYS) mode = "ab+";
    else if (disposition == TRUNCATE_EXISTING) mode = "wb";
    else mode = ((access & GENERIC_WRITE) ? "rb+" : "rb");
    FILE* f = fopen(name, mode);
    if (!f) return INVALID_HANDLE_VALUE;
    return (HANDLE)f;
}

BOOL ReadFile(HANDLE h, void* buf, DWORD toRead, DWORD* read, void*) {
    FILE* f = static_cast<FILE*>(h);
    if (!f) return FALSE;
    size_t n = fread(buf, 1, toRead, f);
    if (read) *read = static_cast<DWORD>(n);
    return TRUE;
}

BOOL WriteFile(HANDLE h, const void* buf, DWORD toWrite, DWORD* written, void*) {
    FILE* f = static_cast<FILE*>(h);
    if (!f) return FALSE;
    size_t n = fwrite(buf, 1, toWrite, f);
    if (written) *written = static_cast<DWORD>(n);
    return TRUE;
}

DWORD SetFilePointer(HANDLE h, LONG dist, LONG* distHigh, DWORD method) {
    FILE* f = static_cast<FILE*>(h);
    if (!f) return INVALID_SET_FILE_POINTER;
    int whence = (method == FILE_BEGIN) ? SEEK_SET : (method == FILE_CURRENT) ? SEEK_CUR : SEEK_END;
    if (fseek(f, dist, whence) != 0) return INVALID_SET_FILE_POINTER;
    long pos = ftell(f);
    if (distHigh) *distHigh = 0;
    return static_cast<DWORD>(pos);
}

DWORD GetFileSize(HANDLE h, DWORD* sizeHigh) {
    FILE* f = static_cast<FILE*>(h);
    if (!f) return INVALID_FILE_SIZE;
    long cur = ftell(f);
    fseek(f, 0, SEEK_END);
    long end = ftell(f);
    fseek(f, cur, SEEK_SET);
    if (sizeHigh) *sizeHigh = 0;
    return static_cast<DWORD>(end);
}

BOOL SetEndOfFile(HANDLE h) {
    FILE* f = static_cast<FILE*>(h);
    if (!f) return FALSE;
    return fflush(f) == 0 ? TRUE : FALSE;
}

BOOL FlushFileBuffers(HANDLE h) {
    FILE* f = static_cast<FILE*>(h);
    return (f && fflush(f) == 0) ? TRUE : FALSE;
}

// ---------------------------------------------------------------------------
// Process
// ---------------------------------------------------------------------------
void ExitProcess(UINT code) { exit(code); }
BOOL TerminateProcess(HANDLE, UINT code) { exit(code); return TRUE; }

} // extern "C"
