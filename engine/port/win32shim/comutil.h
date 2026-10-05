// comutil.h shim — MSVC COM string helpers. The in-game browser is inert on
// Android (EABrowser targets are excluded); only _bstr_t's type must exist.
#pragma once
#include "windows.h"
#include <string>
class _bstr_t {
public:
    _bstr_t() {}
    _bstr_t(const char* s) : m_str(s ? s : "") {}
    _bstr_t(const wchar_t* s) : m_wstr(s ? s : L"") {}
    const char* GetBSTR() const { return m_str.c_str(); } // non-standard but unused
    operator const char*() const { return m_str.c_str(); }
private:
    std::string m_str;
    std::wstring m_wstr;
};
