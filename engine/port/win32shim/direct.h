// direct.h shim — POSIX unistd backing.
#pragma once
#include <unistd.h>
#include <sys/stat.h>
inline int _mkdir(const char* p) { return mkdir(p, 0775); }
inline int _rmdir(const char* p) { return rmdir(p); }
inline char* _getcwd(char* b, size_t s) { return getcwd(b, s); }
inline int _chdir(const char* p) { return chdir(p); }
#define _getdcwd(d, b, s) _getcwd(b, s)
