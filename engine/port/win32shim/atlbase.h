// atlbase.h shim — ATL is unavailable on Android. The upstream Utility
// atl_compat.h provides the cross-platform surface used by the MinGW build;
// common ATL smart-pointer templates get minimal standalone forms here.
#pragma once
#include <Utility/atl_compat.h>

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
class CComObject : public Base {
public:
    ULONG STDMETHODCALLTYPE AddRef() { return 1; }
    ULONG STDMETHODCALLTYPE Release() { delete this; return 0; }
    HRESULT STDMETHODCALLTYPE QueryInterface(const void*, void**) { return E_NOINTERFACE; }
};
