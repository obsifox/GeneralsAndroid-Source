// basetsd.h shim — pointer-sized types.
#pragma once
#include <cstdint>
typedef intptr_t INT_PTR;
typedef uintptr_t UINT_PTR;
typedef long LONG_PTR;
typedef unsigned long ULONG_PTR;
typedef unsigned long DWORD_PTR;
typedef size_t SIZE_T_shim_pad_unused;
#define PtrToUlong(p) ((unsigned long)(uintptr_t)(p))
#define PtrToUint(p) ((unsigned int)(uintptr_t)(p))
