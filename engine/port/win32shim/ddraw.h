// ddraw.h shim — DDS texture files are parsed directly (no DirectDraw at
// runtime); only the header constants and color-key types are needed.
#pragma once
#include "windows.h"
typedef DWORD DDCOLORKEY_shim_unused;
typedef struct _DDCOLORKEY {
    DWORD dwColorSpaceLowValue;
    DWORD dwColorSpaceHighValue;
} DDCOLORKEY, *LPDDCOLORKEY;
#define DDSD_CAPS 0x00000001l
#define DDSD_HEIGHT 0x00000002l
#define DDSD_WIDTH 0x00000004l
#define DDSD_PITCH 0x00000009l
#define DDSD_PIXELFORMAT 0x00001000l
#define DDSD_MIPMAPCOUNT 0x00020000l
#define DDSD_LINEARSIZE 0x00080000l
#define DDSD_DEPTH 0x00800000l
#define DDPF_ALPHAPIXELS 0x00000001l
#define DDPF_ALPHA 0x00000002l
#define DDPF_FOURCC 0x00000004l
#define DDPF_RGB 0x00000040l
#define DDPF_LUMINANCE 0x00020000l
#define DDSCAPS_TEXTURE 0x00001000l
#define DDSCAPS_MIPMAP 0x00400000l
#define DDSCAPS_COMPLEX 0x00000008l
#define DDSCAPS2_CUBEMAP 0x00000200l
#define DDSCAPS2_VOLUME 0x00200000l
// NOTE: DDS_* enums belong to ddsfile.h — do not define them here.
#define MAKEFOURCC(a, b, c, d) ((DWORD)(((BYTE)(a)) | ((BYTE)(b) << 8) | ((BYTE)(c) << 16) | ((BYTE)(d) << 24)))
