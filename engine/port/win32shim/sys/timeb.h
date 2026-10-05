// sys/timeb.h — bionic (Android libc) has no <sys/timeb.h>. MinGW-era engine
// code (PreRTS.h chain) includes it for ftime()/struct timeb. Backed by
// gettimeofday, which is the same wall-clock source.
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source.
#ifndef GENERALS_PORT_SYS_TIMEB_H
#define GENERALS_PORT_SYS_TIMEB_H

#include <sys/time.h>
#include <time.h>

struct timeb {
    time_t time;             /* seconds since epoch */
    unsigned short millitm;  /* milliseconds */
    short timezone;          /* minutes west of GMT (unused here) */
    short dstflag;           /* non-zero if DST in effect (unused here) */
};

static inline int ftime(struct timeb* tb) {
    struct timeval tv;
    if (gettimeofday(&tv, /*tzp=*/nullptr) != 0) return -1;
    if (tb) {
        tb->time = tv.tv_sec;
        tb->millitm = (unsigned short)(tv.tv_usec / 1000);
        tb->timezone = 0;
        tb->dstflag = 0;
    }
    return 0;
}

#define _ftime ftime

#endif /* GENERALS_PORT_SYS_TIMEB_H */
