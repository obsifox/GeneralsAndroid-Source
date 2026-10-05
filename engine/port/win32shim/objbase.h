// objbase.h shim — COM macros for the D3D8 SDK headers. No runtime:
// D3D/DInput entry points are stubs that fail fast (see port/d3d8stub).
#pragma once
#include "unknwn.h"
#include "objidl.h"

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

#define STDAPI EXTERN_C HRESULT STDAPICALLTYPE
#define STDAPI_(type) EXTERN_C type STDAPICALLTYPE
#define DECLARE_INTERFACE(iface) interface iface
#define DECLARE_INTERFACE_(iface, base) interface iface : public base
#define STDMETHOD(name) virtual HRESULT name
#define STDMETHOD_(type, name) virtual type name
#define STDMETHODV(name) virtual HRESULT name
#define STDMETHODV_(type, name) virtual type name
#define THIS_
#define THIS void
#define PURE = 0

inline HRESULT CoInitialize(void*) { return S_OK; }
#define CoInitializeEx(a, b) CoInitialize(a)
inline void CoUninitialize(void) {}
inline HRESULT CoCreateInstance(const void*, const void*, DWORD, const void*, void**) { return E_NOTIMPL; }
#define CLSCTX_INPROC_SERVER 1
#define CLSCTX_ALL 0x17
