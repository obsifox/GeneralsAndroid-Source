// oleauto.h shim — OLE automation is absent on Android; the WOL browser
// chain only needs the BSTR/VARIANT type names to compile.
#pragma once
#include "windows.h"
#include "unknwn.h"
typedef WCHAR* BSTR;
typedef struct tagVARIANT {
    WORD vt;
    union { LONG lVal; BYTE bVal; FLOAT fltVal; void* byref; };
} VARIANT, *LPVARIANT;
inline BSTR SysAllocString(const wchar_t* s) { return (BSTR)s; }
inline void SysFreeString(BSTR) {}
inline UINT SysStringLen(BSTR) { return 0; }
inline HRESULT VariantClear(VARIANT* v) { if (v) v->vt = 0; return S_OK; }
