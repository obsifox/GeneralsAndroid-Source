// comip.h shim — MSVC COM compiler support. The in-game web browser is
// inert on Android; only the type surface must exist for compilation.
#pragma once
#include "windows.h"
#include "comutil.h"
typedef void* LPDISPATCH;
template <typename T> class _com_ptr_t {
public:
    _com_ptr_t() : m_p(nullptr) {}
    T* operator->() const { return m_p; }
private:
    T* m_p;
};
