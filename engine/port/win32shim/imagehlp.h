// imagehlp.h shim — crash-dump helpers do not exist on Android; the crash
// path (WWLib/Except.cpp) is inert on mobile (Android tombstones instead).
#pragma once
#include "windows.h"
typedef struct _IMAGEHLP_SYMBOL *PIMAGEHLP_SYMBOL_shim_unused;
