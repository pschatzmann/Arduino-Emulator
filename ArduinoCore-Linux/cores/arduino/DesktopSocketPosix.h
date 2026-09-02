#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace arduino {

using SocketHandle = int;
constexpr SocketHandle INVALID_SOCKET_HANDLE = -1;
using SocketLength = socklen_t;

class SocketRuntime {
 public:
  SocketRuntime() = default;
  ~SocketRuntime() = default;
};

inline void ensureSocketRuntime() {
  static SocketRuntime runtime;
  (void)runtime;
}

inline void closeSocket(SocketHandle socket) { ::close(socket); }
inline int socketLastError() { return errno; }

inline bool socketWouldBlock(int error) {
  return error == EWOULDBLOCK || error == EAGAIN;
}
inline bool socketConnectionReset(int error) { return error == ECONNRESET; }

inline bool setSocketNonBlocking(SocketHandle socket, bool enabled = true) {
  int flags = fcntl(socket, F_GETFL, 0);
  return flags >= 0 && fcntl(socket, F_SETFL, enabled ? flags | O_NONBLOCK
                                                       : flags & ~O_NONBLOCK) == 0;
}

}  // namespace arduino
