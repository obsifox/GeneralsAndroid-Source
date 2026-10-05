// unknwn.h shim — GUID/IUnknown base for COM-style headers (D3D8 SDK).
// Objects are never instantiated on Android (D3D/DInput stubs fail fast);
// only the type system must exist for compilation.
#pragma once
#include "windows.h"

// GUID is defined in windows.h; re-typedefs here are identical and legal.
typedef GUID IID;
typedef GUID CLSID;
typedef GUID* LPGUID;
typedef const GUID* LPCGUID;

#ifdef __cplusplus
#define EXTERN_C extern "C"
typedef const GUID& REFGUID;
typedef const IID& REFIID;
#else
#define EXTERN_C extern
typedef const GUID REFGUID;
typedef const IID REFIID;
#endif

struct IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
    virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
    virtual ~IUnknown() {}
};

typedef IUnknown* LPUNKNOWN;
#define OPTIONAL

struct IClassFactory : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj) = 0;
    virtual HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock) = 0;
};
