
/*
	SocketImpl.cpp
	Copyright (c) 2025 Phil Schatzmann. All right reserved.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "SocketImpl.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#ifdef __APPLE__
#include <net/route.h>
#include <sys/sysctl.h>
#endif

#include <cstring>

#include "ArduinoLogger.h"

namespace arduino {

const char *SOCKET_IMPL = "SocketImpl";

// checks if we are connected
uint8_t SocketImpl::connected() {
  if (sock < 0) return false;
  char buf[2];
  int result = ::recv(sock, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
  if (result > 0) {
    is_connected = true;
    return true;
  }

  if (result == 0) {
    Logger.info(SOCKET_IMPL, "peer closed connection");
    close();
    is_connected = false;
    return false;
  }

  if (errno == EAGAIN || errno == EWOULDBLOCK) {
    is_connected = true;
    return true;
  }

  int error_code = 0;
  socklen_t error_code_size = sizeof(error_code);
  // int getsockopt(int sockfd, int level, int optname,void *optval, socklen_t
  // *optlen);
  int sockopt_result =
      getsockopt(sock, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
  if (sockopt_result != 0) {
    char msg[50];
    sprintf(msg, "%d", sockopt_result);
    Logger.debug(SOCKET_IMPL, "getsockopt->", msg);
  }

  if (sockopt_result == 0 && error_code == 0) {
    is_connected = true;
    return true;
  }

  close();
  is_connected = false;
  return false;
}

// opens a conection
int SocketImpl::connect(const char *address, uint16_t port) {
  return connect(address, port, -1);
}

int SocketImpl::connect(const char *address, uint16_t port, int32_t timeout_ms) {
  if (sock >= 0) {
    close();
  }

  if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    is_connected = false;
    Logger.error(SOCKET_IMPL, "could not create socket");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  const char *address_ip = address;
  hostent *host_entry = gethostbyname(address);  // find host information
  if (host_entry != nullptr) {
    address_ip =
        inet_ntoa(*((struct in_addr *)
                        host_entry->h_addr_list[0]));  // Convert into IP string
  }

  // Convert IPv4 and IPv6 addresses from text to binary form
  if (::inet_pton(AF_INET, address_ip, &serv_addr.sin_addr) <= 0) {
    close();
    Logger.error(SOCKET_IMPL, "invalid address");
    return -2;
  }

  if (timeout_ms < 0) {
    if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      close();
      Logger.error(SOCKET_IMPL, "could not connect");
      return -3;
    }
  } else {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
      flags = 0;
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
      close();
      Logger.error(SOCKET_IMPL, "could not set nonblocking connect");
      return -3;
    }

    int result = ::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (result < 0) {
      if (errno != EINPROGRESS && errno != EWOULDBLOCK) {
        fcntl(sock, F_SETFL, flags);
        close();
        Logger.error(SOCKET_IMPL, "could not connect");
        return -3;
      }

      fd_set writefds;
      FD_ZERO(&writefds);
      FD_SET(sock, &writefds);

      fd_set errorfds;
      FD_ZERO(&errorfds);
      FD_SET(sock, &errorfds);

      timeval timeout {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000,
      };

      result = select(sock + 1, nullptr, &writefds, &errorfds, &timeout);
      if (result == 0) {
        fcntl(sock, F_SETFL, flags);
        close();
        Logger.error(SOCKET_IMPL, "connect timeout");
        return -4;
      }

      if (result < 0) {
        fcntl(sock, F_SETFL, flags);
        close();
        Logger.error(SOCKET_IMPL, "select failed during connect");
        return -3;
      }

      int error_code = 0;
      socklen_t error_len = sizeof(error_code);
      if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error_code, &error_len) != 0 || error_code != 0) {
        fcntl(sock, F_SETFL, flags);
        close();
        Logger.error(SOCKET_IMPL, "could not connect");
        return -3;
      }
    }

    fcntl(sock, F_SETFL, flags);
  }

  is_connected = true;
  Logger.info(SOCKET_IMPL, "connected!");
  return 1;
}

// send the data via the socket - returns the number of characters written or
// -1=>Error
size_t SocketImpl::write(const uint8_t *str, size_t len) {
  Logger.debug(SOCKET_IMPL, "write");
  return ::send(sock, str, len, MSG_NOSIGNAL);
}

// provides the available bytes
size_t SocketImpl::available() {
  if (sock < 0) {
    is_connected = false;
    return 0;
  }

  int bytes_available;
  if (ioctl(sock, FIONREAD, &bytes_available) != 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      close();
      is_connected = false;
    }
    return 0;
  }

  if (bytes_available == 0 && !connected()) {
    return 0;
  }

  char msg[50];
  sprintf(msg, "%d", bytes_available);
  Logger.debug(SOCKET_IMPL, "available->", msg);
  return bytes_available;
}

// direct read
size_t SocketImpl::read(uint8_t *buffer, size_t len) {
  if (sock < 0) {
    is_connected = false;
    return static_cast<size_t>(-1);
  }

  ssize_t result = ::recv(sock, buffer, len, MSG_DONTWAIT | MSG_NOSIGNAL);
  if (result == 0) {
    Logger.info(SOCKET_IMPL, "read EOF");
    close();
    is_connected = false;
    return static_cast<size_t>(-1);
  }

  if (result < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      close();
      is_connected = false;
    }
    return static_cast<size_t>(-1);
  }

  char lenStr[80];
  sprintf(lenStr, "%ld -> %ld", len, result);
  Logger.debug(SOCKET_IMPL, "read->", lenStr);
  return result;
}

// peeks one character
int SocketImpl::peek() {
  Logger.debug(SOCKET_IMPL, "peek");
  char buf[1];
  int result = ::recv(sock, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
  return result > 0 ? buf[0] : -1;
}

void SocketImpl::close() {
  Logger.info(SOCKET_IMPL, "close");
  if (sock >= 0) {
    ::close(sock);
    sock = -1;
  }
  is_connected = false;
}

// Linux-compatible implementation: parse /proc/net/route for default interface
#if defined(__linux__)
static char *defaultInterface() {
  FILE *f;
  char line[256], *p, *c;

  f = fopen("/proc/net/route", "r");
  if (!f) {
    return nullptr;
  }

  // Skip the header line
  if (!fgets(line, sizeof(line), f)) {
    fclose(f);
    return nullptr;
  }

  while (fgets(line, sizeof(line), f)) {
    p = strtok(line, " \t");
    c = strtok(NULL, " \t");

    if (p != NULL && c != NULL) {
      if (strcmp(c, "00000000") == 0) {
        static char defaultInterface[IF_NAMESIZE];
        strncpy(defaultInterface, p, IF_NAMESIZE - 1);
        defaultInterface[IF_NAMESIZE - 1] = '\0';
        Logger.info("Default network interface is ", p);
        fclose(f);
        return defaultInterface;
      }
    }
  }
  fclose(f);
  return nullptr;
}
#elif defined(__APPLE__)
// macOS/BSD implementation using sysctl
static char *defaultInterface() {
  int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_FLAGS, RTF_GATEWAY};
  size_t needed = 0;

  if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0 || needed == 0)
    return nullptr;

  char *buf = (char *)malloc(needed);
  if (!buf) return nullptr;

  if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
    free(buf);
    return nullptr;
  }

  static char ifname[IF_NAMESIZE];
  char *result = nullptr;

  for (char *p = buf; p < buf + needed;) {
    struct rt_msghdr *rtm = (struct rt_msghdr *)p;
    struct sockaddr *sa = (struct sockaddr *)(rtm + 1);
    if (sa->sa_family == AF_INET) {
      struct sockaddr_in *sin = (struct sockaddr_in *)sa;
      if (sin->sin_addr.s_addr == INADDR_ANY) {
        if (if_indextoname(rtm->rtm_index, ifname) != nullptr) {
          Logger.info("Default network interface is ", ifname);
          result = ifname;
        }
        break;
      }
    }
    p += rtm->rtm_msglen;
  }

  free(buf);
  return result;
}
#else
// Stub for other platforms
static char *defaultInterface() {
  return nullptr;
}
#endif

// determines the IP Adress
const char *SocketImpl::getIPAddress() {
  Logger.info(SOCKET_IMPL, "getIPAddress");
  const char *valid[] = {"eth0", "en0", "en1", defaultInterface(), nullptr};
  return getIPAddress(valid);
}

// determines the IP Adress
const char *SocketImpl::getIPAddress(const char *validEntries[]) {
  Logger.info(SOCKET_IMPL, "getIPAddress-1");
  struct ifaddrs *interfaces = NULL;
  struct ifaddrs *temp_addr = NULL;
  char *resultAddress = nullptr;
  int success = 0;
  // retrieve the current interfaces - returns 0 on success
  success = getifaddrs(&interfaces);
  if (success == 0) {
    // Loop through linked list of interfaces
    temp_addr = interfaces;
    while (temp_addr != NULL) {
      if (temp_addr->ifa_addr->sa_family == AF_INET &&
          (temp_addr->ifa_flags & IFF_UP)) {
        // check if ifa_name is in the list of relevant names
        bool isValid = false;
        for (int j = 0; validEntries[j] != nullptr; j++) {
          if (strcmp(temp_addr->ifa_name, validEntries[j]) == 0) {
            isValid = true;
            break;
          }
        }
        if (isValid) {
          auto addr = ((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr;
          resultAddress = inet_ntoa(addr);
          break;
        }
      }
      temp_addr = temp_addr->ifa_next;
    }
  }
  // Free memory
  freeifaddrs(interfaces);
  return resultAddress;
}

}  // namespace arduino
