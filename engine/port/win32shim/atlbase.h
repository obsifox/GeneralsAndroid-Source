// atlbase.h shim — ATL is unavailable on Android. Minimal COM plumbing for
// the WOLBrowser/FEBDispatch chain (the browser itself is inert on mobile).
#pragma once
#include <Utility/atl_compat.h>
#include <unknwn.h>
#include <oleauto.h>

struct CComSingleThreadModel
{
    static DWORD STDMETHODCALLTYPE incremented(volatile DWORD* v) { return ++*v; }
    static DWORD STDMETHODCALLTYPE decremented(volatile DWORD* v) { return --*v; }
};

template <typename T>
class CComObjectRootEx
{
public:
    ULONG STDMETHODCALLTYPE InternalAddRef() { return 1; }
    ULONG STDMETHODCALLTYPE InternalRelease() { return 0; }
    HRESULT STDMETHODCALLTYPE FinalConstruct() { return S_OK; }
    void STDMETHODCALLTYPE FinalRelease() {}
};

template <typename T>
class CComCoClass
{
public:
    HRESULT STDMETHODCALLTYPE FinalConstruct() { return S_OK; }
};

class CComModule
{
public:
    HRESULT STDMETHODCALLTYPE Init(void*, void*, DWORD*) { return S_OK; }
    void STDMETHODCALLTYPE Term() {}
};
extern CComModule _Module;

#define BEGIN_COM_MAP(T) \
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) { \
        if (!ppv) return E_POINTER; \
        *ppv = nullptr;
#define COM_INTERFACE_ENTRY(x) \
        if (riid == __uuidof_shim_get(x)) { *ppv = static_cast<x*>(this); AddRef(); return S_OK; }
#define COM_INTERFACE_ENTRY_AGGREGATE(iid, agg) \
        if (riid == iid) { *ppv = agg; AddRef(); return S_OK; }
#define END_COM_MAP() \
        return E_NOINTERFACE; \
    }

// NOTE: the port does not use real type libraries; the map above only needs
// to compile. __uuidof_shim_get returns a dummy GUID per type name.
template <typename T> inline const GUID& __uuidof_shim_get_impl() {
    static const GUID g = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
    return g;
}
#define __uuidof_shim_get(x) __uuidof_shim_get_impl<x>()

template <typename T>
class CComPtr {
public:
    CComPtr() : m_p(nullptr) {}
    ~CComPtr() { if (m_p) m_p->Release(); }
    T* operator->() const { return m_p; }
    T** operator&() { return &m_p; }
    operator T*() const { return m_p; }
    T* m_p;
};

template <typename Base>
class CComObject : public Base
{
public:
    ULONG STDMETHODCALLTYPE AddRef() { return 1; }
    ULONG STDMETHODCALLTYPE Release() { delete this; return 0; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void**) { return E_NOINTERFACE; }
};
