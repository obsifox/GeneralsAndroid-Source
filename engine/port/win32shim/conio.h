// conio.h shim — Windows console I/O has no Android counterpart; the only
// includer (WWLib/Except.cpp crash path) never runs these on mobile.
#pragma once
inline int _kbhit(void) { return 0; }
inline int getch(void) { return 0; }
inline int _getch(void) { return 0; }
inline int putch(int c) { return c; }
inline int _putch(int c) { return c; }
