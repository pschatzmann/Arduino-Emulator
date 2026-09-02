#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define INPUT ARDUINO_WINDOWS_INPUT
#include <winsock2.h>
#include <ws2tcpip.h>
#undef INPUT
#undef INADDR_NONE

#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif

namespace arduino {

using SocketHandle = SOCKET;
constexpr SocketHandle INVALID_SOCKET_HANDLE = INVALID_SOCKET;
using SocketLength = int;

class SocketRuntime {
 public:
  SocketRuntime() {
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
  }
  ~SocketRuntime() { WSACleanup(); }
};

inline void ensureSocketRuntime() {
  static SocketRuntime runtime;
  (void)runtime;
}

inline void closeSocket(SocketHandle socket) { closesocket(socket); }
inline int socketLastError() { return WSAGetLastError(); }
inline bool socketWouldBlock(int error) { return error == WSAEWOULDBLOCK; }

inline bool setSocketNonBlocking(SocketHandle socket, bool enabled = true) {
  u_long mode = enabled ? 1UL : 0UL;
  return ioctlsocket(socket, FIONBIO, &mode) == 0;
}

}  // namespace arduino
