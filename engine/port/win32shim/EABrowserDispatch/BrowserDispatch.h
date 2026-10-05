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

template <typename Class, typename Iface, const GUID* Iid>
class FEBDispatch
{
public:
    virtual ~FEBDispatch() {}
};
