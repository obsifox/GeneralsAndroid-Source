// atlcom.h shim — ATL COM classes are unavailable on Android; the WOL
// browser chain is inert and only needs the type surface to compile.
#pragma once
#include "atlbase.h"
#include "unknwn.h"
template <typename T>
class CComObjectRootEx { public: ULONG STDMETHODCALLTYPE AddRef() { return 1; } ULONG STDMETHODCALLTYPE Release() { return 0; } };
