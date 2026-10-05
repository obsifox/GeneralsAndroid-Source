// objidl.h shim — stream/storage interfaces referenced by the D3DX headers.
// No runtime on Android: only the type system exists for compilation.
#pragma once
#include "unknwn.h"

struct ISequentialStream : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead) = 0;
    virtual HRESULT STDMETHODCALLTYPE Write(const void* pv, ULONG cb, ULONG* pcbWritten) = 0;
};

struct IStream : public ISequentialStream
{
    virtual HRESULT STDMETHODCALLTYPE Seek(LONGLONG dlibMove, DWORD dwOrigin, ULONGLONG* plibNewPosition) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULONGLONG libNewSize) = 0;
};

struct IMalloc : public IUnknown
{
    virtual LPVOID STDMETHODCALLTYPE Alloc(SIZE_T cb) = 0;
    virtual LPVOID STDMETHODCALLTYPE Realloc(void* pv, SIZE_T cb) = 0;
    virtual void STDMETHODCALLTYPE Free(void* pv) = 0;
    virtual SIZE_T STDMETHODCALLTYPE GetSize(void* pv) = 0;
};

struct IStorage : public IUnknown
{
};

typedef void* LPDISPATCH;
typedef void* LPUNKNOWN_shim_alias_;
