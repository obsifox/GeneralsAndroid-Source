// AndroidMain.cpp — application entry point for the Android port.
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
//
// Mirrors GeneralsMD/Code/Main/WinMain.cpp (excluded from this build) minus
// the Windows window/class/COM parts: critical section globals, memory
// manager, command line, version, single-instance guard, then GameMain().
// CreateGameEngine() is provided here exactly as WinMain.cpp did.
//
// Master Prompt §35: workarounds documented at the site of each hack.

#include "PreRTS.h"

#include "Common/CommandLine.h"
#include "Common/CriticalSection.h"
#include "Common/GlobalData.h"
#include "Common/GameEngine.h"
#include "Common/GameMemory.h"
#include "Common/MessageStream.h"
#include "Common/PlayerList.h"
#include "Common/Registry.h"
#include "Common/StackDump.h"
#include "Common/version.h"
#include "GameClient/ClientInstance.h"
#include "GameLogic/GameLogic.h"
#include "Common/FramePacer.h"

#include "AndroidMain.h"
#include "AndroidGameEngine.h"
#include "BuildVersion.h"
#include "GeneratedVersion.h"

#include "android_log_shim.h"
#include <atomic>
#include <string>
#include <thread>

#ifndef VERSION_STRING_PORT
#define VERSION_STRING_PORT "1.04"
#endif
#ifndef UPSTREAM_PIN
#define UPSTREAM_PIN "f8ba7eb"
#endif
#define TAG "generals-engine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// ---------------------------------------------------------------------------
// Engine status bridge (polled by JNI / diagnostics UI)
// ---------------------------------------------------------------------------
namespace generals {

enum class EnginePhase : int {
    Idle = 0,
    Booting = 1,
    MemoryInit = 2,
    CommandLine = 3,
    EngineInit = 4,
    Running = 5,
    Exited = 6,
    Failed = 7,
};

static std::atomic<EnginePhase> g_phase{EnginePhase::Idle};
static std::string g_phaseDetail;
static std::thread g_engineThread;

static void setPhase(EnginePhase p, const char* detail) {
    g_phase.store(p);
    g_phaseDetail = detail ? detail : "";
    LOGI("[phase %d] %s", static_cast<int>(p), detail ? detail : "");
}

const char* enginePhaseName(EnginePhase p) {
    switch (p) {
        case EnginePhase::Idle: return "idle";
        case EnginePhase::Booting: return "booting";
        case EnginePhase::MemoryInit: return "memory-init";
        case EnginePhase::CommandLine: return "command-line";
        case EnginePhase::EngineInit: return "engine-init";
        case EnginePhase::Running: return "running";
        case EnginePhase::Exited: return "exited";
        case EnginePhase::Failed: return "failed";
    }
    return "unknown";
}

} // namespace generals

// ---------------------------------------------------------------------------
// Critical sections + memory manager, exactly as WinMain does
// ---------------------------------------------------------------------------
// Necessary to allow memory managers and such to have useful critical sections
static CriticalSection critSec1, critSec2, critSec3, critSec4, critSec5;

namespace generals {

static void androidEngineThread(std::string gameDataDir);

// CreateGameEngine ===========================================================
/** Create the Android game engine (Win32GameEngine plus port overrides). */
//=============================================================================
GameEngine *CreateGameEngine()
{
    AndroidGameEngine *engine;
    engine = NEW AndroidGameEngine;
    engine->setIsActive(TRUE);
    return engine;
}

// GameMain ===================================================================
/** The main entry loop — upstream GameMain.cpp keeps this commented because
    the host owns the entry point; on Android that host is us. */
//=============================================================================
static int GameMain()
{
    int exitcode = 0;

    // initialize the game engine using factory function
    TheFramePacer = new FramePacer();
    if (TheFramePacer) {
        TheFramePacer->enableFramesPerSecondLimit(TRUE);
    }

    TheGameEngine = CreateGameEngine();
    if (TheGameEngine == nullptr) {
        setPhase(EnginePhase::Failed, "CreateGameEngine failed");
        return 1;
    }

    setPhase(EnginePhase::EngineInit, "TheGameEngine->init()");
    TheGameEngine->init();

    // run it
    setPhase(EnginePhase::Running, "TheGameEngine->execute()");
    TheGameEngine->execute();

    // since execute() returned, we are exiting the game
    exitcode = 0;
    LOGI("GameMain: engine exited");

    if (TheFramePacer) {
        delete TheFramePacer;
        TheFramePacer = nullptr;
    }
    if (TheGameEngine) {
        delete TheGameEngine;
        TheGameEngine = nullptr;
    }

    setPhase(EnginePhase::Exited, "GameMain returned");
    return exitcode;
}

static void androidEngineThread(std::string gameDataDir)
{
    setPhase(EnginePhase::Booting, gameDataDir.c_str());

    try {
        // Same critical section globals WinMain wires up before anything else.
        TheAsciiStringCriticalSection = &critSec1;
        TheUnicodeStringCriticalSection = &critSec2;
        TheDmaCriticalSection = &critSec3;
        TheMemoryPoolCriticalSection = &critSec4;
        TheDebugLogCriticalSection = &critSec5;

        // initialize the memory manager early
        setPhase(EnginePhase::MemoryInit, "initMemoryManager()");
        initMemoryManager();

        // command line: engine options come from settings; keep minimal
        setPhase(EnginePhase::CommandLine, "parseCommandLineForStartup()");
        CommandLine::parseCommandLineForStartup();

        // working directory: the engine opens Data/... relative to CWD.
        // §35 workaround: chdir is per-process on Android and allowed inside
        // the app sandbox — this is how the engine finds the imported files.
        if (!gameDataDir.empty()) {
            if (chdir(gameDataDir.c_str()) != 0) {
                LOGE("chdir(%s) failed: errno=%d", gameDataDir.c_str(), errno);
            }
        }

        // Set up version info
        TheVersion = NEW Version;
        if (TheVersion) {
            TheVersion->setVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNUM, VERSION_LOCALBUILDNUM,
                AsciiString(VERSION_BUILDUSER), AsciiString(VERSION_BUILDLOC),
                AsciiString(__TIME__), AsciiString(__DATE__));
        }

        // Single-instance guard: on Android the OS guarantees one process per
        // app, so the Win32 mutex dance is unnecessary. Keep the call for the
        // debug log parity but never bail.
        rts::ClientInstance::initialize();

        // run the game main loop
        int exitcode = GameMain();
        LOGI("engine thread done, exitcode=%d", exitcode);

        if (TheVersion) {
            delete TheVersion;
            TheVersion = nullptr;
        }

        shutdownMemoryManager();

        // reset critical section globals like WinMain's epilogue
        TheUnicodeStringCriticalSection = nullptr;
        TheDmaCriticalSection = nullptr;
        TheMemoryPoolCriticalSection = nullptr;
    }
    catch (const std::exception& e) {
        LOGE("engine exception: %s", e.what());
        setPhase(EnginePhase::Failed, e.what());
    }
    catch (...) {
        LOGE("engine exception: unknown");
        setPhase(EnginePhase::Failed, "unknown exception");
    }
}

// ---------------------------------------------------------------------------
// Public bootstrap API (called by generals_jni.cpp)
// ---------------------------------------------------------------------------
bool androidStartEngine(const std::string& gameDataDir)
{
    if (g_engineThread.joinable()) {
        // already running
        return true;
    }
    setPhase(EnginePhase::Booting, "spawning engine thread");
    g_engineThread = std::thread(androidEngineThread, gameDataDir);
    // NOTE: intentionally not detached — join on shutdown to avoid leaks.
    return true;
}

void androidShutdownEngine()
{
    if (TheGameEngine) {
        TheGameEngine->setQuitting(TRUE);
    }
    if (g_engineThread.joinable()) {
        g_engineThread.join();
    }
}

int androidEngineStatus(char* out, int outLen)
{
    if (!out || outLen <= 0) return 0;
    EnginePhase p = g_phase.load();
    int n = snprintf(out, outLen, "phase=%s detail=%s",
                     enginePhaseName(p),
                     g_phaseDetail.c_str());
    return n > 0 ? n : 0;
}

const char* engineVersionString()
{
    return "engine " VERSION_STRING_PORT " (upstream " UPSTREAM_PIN ")";
}

} // namespace generals
