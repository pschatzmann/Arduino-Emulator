/*
	SocketImplWindows.cpp
	Copyright (c) 2025 Phil Schatzmann. All right reserved.

	Windows/Winsock implementation of SocketImpl. The original POSIX
	implementation remains in SocketImpl.cpp.
*/

#include "SocketImpl.h"

#if ARDUINO_EMULATOR_WINDOWS

#include <algorithm>
#include <cstring>
#include <vector>

#include <iphlpapi.h>

#include "ArduinoLogger.h"

namespace arduino {

const char *SOCKET_IMPL = "SocketImpl";

uint8_t SocketImpl::connected() {
  ensureSocketRuntime();
  if (sock == INVALID_SOCKET_HANDLE) return false;

  char buffer[1];
  int result = ::recv(sock, buffer, 1, MSG_PEEK);
  if (result > 0) return is_connected = true;
  if (result == 0) return is_connected = false;
  if (socketWouldBlock(socketLastError())) return is_connected;
  return is_connected = false;
}

int SocketImpl::connect(const char *address, uint16_t port) {
  ensureSocketRuntime();

  addrinfo hints{};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  addrinfo *resolved = nullptr;
  char service[8];
  snprintf(service, sizeof(service), "%u", port);

  if (::getaddrinfo(address, service, &hints, &resolved) != 0 || !resolved) {
    Logger.error(SOCKET_IMPL, "invalid address");
    return -2;
  }

  sock = ::socket(resolved->ai_family, resolved->ai_socktype,
                  resolved->ai_protocol);
  if (sock == INVALID_SOCKET_HANDLE) {
    freeaddrinfo(resolved);
    Logger.error(SOCKET_IMPL, "could not create socket");
    return -1;
  }

  std::memcpy(&serv_addr, resolved->ai_addr,
              std::min(sizeof(serv_addr),
                       static_cast<size_t>(resolved->ai_addrlen)));
  int result = ::connect(sock, resolved->ai_addr,
                         static_cast<SocketLength>(resolved->ai_addrlen));
  freeaddrinfo(resolved);

  if (result < 0) {
    close();
    Logger.error(SOCKET_IMPL, "could not connect");
    return -3;
  }

  setSocketNonBlocking(sock);
  is_connected = true;
  Logger.info(SOCKET_IMPL, "connected!");
  return 1;
}

size_t SocketImpl::write(const uint8_t *data, size_t length) {
  ensureSocketRuntime();
  return ::send(sock, reinterpret_cast<const char *>(data),
                static_cast<int>(length), 0);
}

size_t SocketImpl::available() {
  ensureSocketRuntime();
  u_long bytes = 0;
  ioctlsocket(sock, FIONREAD, &bytes);
  return bytes;
}

size_t SocketImpl::read(uint8_t *buffer, size_t length) {
  ensureSocketRuntime();
  int result = ::recv(sock, reinterpret_cast<char *>(buffer),
                      static_cast<int>(length), 0);
  if (result < 0 && socketWouldBlock(socketLastError())) return 0;
  return result > 0 ? static_cast<size_t>(result) : 0;
}

int SocketImpl::peek() {
  ensureSocketRuntime();
  char buffer[1];
  int result = ::recv(sock, buffer, 1, MSG_PEEK);
  return result > 0 ? static_cast<unsigned char>(buffer[0]) : -1;
}

void SocketImpl::close() {
  Logger.info(SOCKET_IMPL, "close");
  if (sock != INVALID_SOCKET_HANDLE) closeSocket(sock);
  sock = INVALID_SOCKET_HANDLE;
  is_connected = false;
}

const char *SocketImpl::getIPAddress() {
  return getIPAddress(nullptr);
}

const char *SocketImpl::getIPAddress(const char *validEntries[]) {
  static char address[INET_ADDRSTRLEN] = "127.0.0.1";
  ensureSocketRuntime();

  ULONG buffer_length = 0;
  if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr,
                           &buffer_length) != ERROR_BUFFER_OVERFLOW) {
    return address;
  }

  std::vector<unsigned char> buffer(buffer_length);
  auto *adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());
  if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, nullptr, adapters,
                           &buffer_length) != NO_ERROR) {
    return address;
  }

  for (auto *adapter = adapters; adapter != nullptr; adapter = adapter->Next) {
    if (adapter->OperStatus != IfOperStatusUp) continue;

    bool is_valid = validEntries == nullptr;
    for (size_t i = 0; !is_valid && validEntries[i] != nullptr; ++i) {
      if (std::strcmp(adapter->AdapterName, validEntries[i]) == 0) {
        is_valid = true;
        break;
      }

      int length = WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1,
                                       nullptr, 0, nullptr, nullptr);
      if (length <= 0) continue;
      std::vector<char> friendly_name(length);
      WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1,
                          friendly_name.data(), length, nullptr, nullptr);
      if (std::strcmp(friendly_name.data(), validEntries[i]) == 0) {
        is_valid = true;
        break;
      }
    }
    if (!is_valid) continue;

    for (auto *entry = adapter->FirstUnicastAddress; entry != nullptr;
         entry = entry->Next) {
      if (entry->Address.lpSockaddr->sa_family == AF_INET) {
        auto *ipv4 = reinterpret_cast<sockaddr_in *>(entry->Address.lpSockaddr);
        if (inet_ntop(AF_INET, &ipv4->sin_addr, address, sizeof(address))) {
          return address;
        }
      }
    }
  }
  return address;
}

}  // namespace arduino

#endif  // ARDUINO_EMULATOR_WINDOWS
