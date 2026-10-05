// io.h shim — lowio mapping.
#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
inline int _access(const char* p, int m) { return access(p, m == 2 ? W_OK : (m == 4 ? R_OK : F_OK)); }
#define _open open
#define _close close
#define _read read
#define _write write
#define _lseek lseek
#define _unlink unlink
#define _commit fsync
typedef long _off_t_shim_unused;
#define _setmode(f, m) 0
#define _O_BINARY 0
#define _O_TEXT 0
