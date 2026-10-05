// EABrowserDispatch/BrowserDispatch.h shim — the EA browser-dispatch SDK
// was never open-sourced (like osdep.h). The in-game web browser is inert on
// Android; only the FEBDispatch seam and the dispatch IID must exist so the
// WOLBrowser headers compile.
#pragma once
#include "windows.h"
#include "unknwn.h"

#ifndef IID_IBrowserDispatch
DEFINE_GUID(IID_IBrowserDispatch, 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01);
#endif

// NOTE: FEBDispatch<Class, Iface, IID> template is provided upstream by
// WOLBrowser/FEBDispatch.h — do not redefine it here.
class IBrowserDispatch
{
public:
    virtual ~IBrowserDispatch() {}
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void**) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
    virtual ULONG STDMETHODCALLTYPE Release() = 0;
};
