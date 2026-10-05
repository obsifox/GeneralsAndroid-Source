// winsock2.h — BSD sockets shim behind the Winsock interface.
// The engine's networking layer calls Winsock directly; on Android the BSD
// socket API is nearly identical, so we map names and error codes.
// GPLv3-or-later. Part of obsifox/GeneralsAndroid-Source. Master Prompt §35.

#pragma once

#if defined(_MSC_VER) || defined(__MINGW32__)
#error "winsock shim must never be used on a real Windows compiler"
#endif

#include "windows.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

typedef int SOCKET;
typedef unsigned long u_long;

// Winsock type aliases — GameSpy's platform headers reference these names
// on every platform; on BSD they map to the standard structs.
typedef struct in_addr IN_ADDR, *PIN_ADDR, *LPIN_ADDR;
typedef struct hostent HOSTENT, *LPHOSTENT;
typedef struct sockaddr SOCKADDR, *PSOCKADDR, *LPSOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN, *PSOCKADDR_IN, *LPSOCKADDR_IN;
typedef struct sockaddr_in6 SOCKADDR_IN6, *PSOCKADDR_IN6, *LPSOCKADDR_IN6;
typedef struct servent SERVENT, *PSERVENT, *LPSERVENT;
typedef struct protoent PROTOENT, *PPROTOENT, *LPPROTOENT;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#define SOCKET_ERROR (-1)

#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINPROGRESS EINPROGRESS
#define WSAEINTR EINTR
#define WSAEINVAL EINVAL
#define WSAEADDRINUSE EADDRINUSE
#define WSAENETDOWN ENETDOWN
#define WSAENETRESET ENETRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAECONNRESET ECONNRESET
#define WSAENOBUFS ENOBUFS
#define WSAENOTCONN ENOTCONN
#define WSAESHUTDOWN ESHUTDOWN
#define WSAETIMEDOUT ETIMEDOUT
#define WSAECONNREFUSED ECONNREFUSED
#define WSAEHOSTUNREACH EHOSTUNREACH
#define WSAEMSGSIZE EMSGSIZE
#define WSANOTINITIALISED EPERM

#define SD_RECEIVE SHUT_RD
#define SD_SEND SHUT_WR
#define SD_BOTH SHUT_RDWR

#define WSA_FLAG_OVERLAPPED 0x01
#define FROM_PROTOCOL_INFO -1

inline int WSAGetLastError(void) { return errno; }
inline void WSASetLastError(int err) { errno = err; }
inline int WSAStartup(unsigned short, void*) { return 0; }
inline int WSACleanup(void) { return 0; }
inline int closesocket(SOCKET s) { return close(s); }
inline int ioctlsocket(SOCKET s, long cmd, unsigned long* argp) {
    if (cmd == 0x8004667Eu) { // FIONBIO
        int nonblock = (argp && *argp) ? 1 : 0;
        return fcntl(s, F_SETFL, nonblock ? O_NONBLOCK : 0);
    }
    return fcntl(s, F_GETFL) >= 0 ? 0 : -1;
}
#define FIONBIO 0x8004667Eu
#define WSAIoctl ioctlsocket

// host-to-network helpers come from arpa/inet.h on POSIX (same names).

struct WSAEVENT_shim { void* unused; };
typedef struct WSAEVENT_shim* WSAEVENT;
#define WSA_INVALID_EVENT ((WSAEVENT)0)
inline WSAEVENT WSACreateEvent(void) { return WSA_INVALID_EVENT; }
inline BOOL WSASetEvent(WSAEVENT) { return FALSE; }
inline BOOL WSACloseEvent(WSAEVENT) { return FALSE; }
inline int WSAEventSelect(SOCKET, WSAEVENT, long) { return 0; }
#define FD_READ_BIT 0
#define FD_WRITE_BIT 1
#define FD_READ (1 << FD_READ_BIT)
#define FD_WRITE (1 << FD_WRITE_BIT)
#define FD_CLOSE (1 << 5)
inline int WSAEnumNetworkEvents(SOCKET, WSAEVENT, void*) { return 0; }
