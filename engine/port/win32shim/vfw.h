// vfw.h shim — Video-for-Windows AVI capture is a Windows-only dev feature;
// the movie path is replaced on Android (Bink loader fails fast, no AVI).
#pragma once
#include "windows.h"
typedef struct AVIFileShim_* PAVIFILE;
typedef struct AVIStreamShim_* PAVISTREAM;
typedef struct AVICompressShim_ {
    DWORD fccType;
    DWORD fccHandler;
    DWORD dwFlags;
    DWORD dwQuality;
} AVICOMPRESSOPTIONS, *LPAVICOMPRESSOPTIONS;
typedef struct AVIStreamInfoShim_ {
    DWORD fccType;
    DWORD fccHandler;
    DWORD dwFlags;
    DWORD dwCaps;
    WORD wPriority;
    WORD wLanguage;
    DWORD dwScale;
    DWORD dwRate;
    DWORD dwStart;
    DWORD dwLength;
    DWORD dwInitialFrames;
    DWORD dwSuggestedBufferSize;
    DWORD dwQuality;
    DWORD dwSampleSize;
    RECT rcFrame;
    DWORD dwEditCount;
    DWORD dwFormatChangeCount;
    char szName[64];
} AVISTREAMINFO, *LPAVISTREAMINFO;
#define AVIIF_KEYFRAME 0x00000010L
#define streamtypeVIDEO 0x73646976 // 'vids'
#define streamtypeAUDIO 0x73647561 // 'auds'
#define mmioFOURCC(a, b, c, d) ((DWORD)(((BYTE)(a)) | ((BYTE)(b) << 8) | ((BYTE)(c) << 16) | ((BYTE)(d) << 24)))
inline void AVIFileInit(void) {}
inline void AVIFileExit(void) {}
inline HRESULT AVIFileOpen(PAVIFILE*, const char*, UINT, void*) { return E_FAIL; }
inline HRESULT AVIFileCreateStream(PAVIFILE, PAVISTREAM*, AVISTREAMINFO*) { return E_FAIL; }
inline HRESULT AVIMakeCompressedStream(PAVISTREAM*, PAVISTREAM, AVICOMPRESSOPTIONS*, void*) { return E_FAIL; }
inline HRESULT AVIStreamSetFormat(PAVISTREAM, LONG, void*, LONG) { return E_FAIL; }
inline HRESULT AVIStreamWrite(PAVISTREAM, LONG, LONG, void*, LONG, DWORD, LONG*, LONG*) { return E_FAIL; }
inline ULONG AVIFileRelease(PAVIFILE) { return 0; }
inline ULONG AVIStreamRelease(PAVISTREAM) { return 0; }
