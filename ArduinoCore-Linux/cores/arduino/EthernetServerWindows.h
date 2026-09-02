#pragma once

#include "DesktopSocket.h"
#include "Ethernet.h"
#include "api/Server.h"
#include "SignalHandler.h"

namespace arduino {

class EthernetServer : public Server {
 private:
  uint16_t _port;
  SocketHandle server_fd = INVALID_SOCKET_HANDLE;
  struct sockaddr_in server_addr;
  int _status = wl_status_t::WL_DISCONNECTED;
  bool is_blocking = false;
  bool _noDelay = false;

  static std::vector<EthernetServer*>& active_servers() {
    static std::vector<EthernetServer*> servers;
    return servers;
  }
  static void cleanupAll(int) {
    for (auto* server : active_servers()) {
      if (server && server->server_fd != INVALID_SOCKET_HANDLE) {
        shutdown(server->server_fd, SHUT_RDWR);
        closeSocket(server->server_fd);
        server->server_fd = INVALID_SOCKET_HANDLE;
      }
    }
  }

 public:
  EthernetServer(int port = 80) : _port(port) {
    static bool signal_registered = false;
    if (!signal_registered) {
      SignalHandler::registerHandler(SIGINT, cleanupAll);
      SignalHandler::registerHandler(SIGTERM, cleanupAll);
      signal_registered = true;
    }
  }
  ~EthernetServer() {
    stop();
    auto& servers = active_servers();
    auto it = std::find(servers.begin(), servers.end(), this);
    if (it != servers.end()) servers.erase(it);
  }

  void begin() { begin(_port); }
  void begin(int port) { begin_(port); }
  void stop() {
    if (server_fd != INVALID_SOCKET_HANDLE) {
      struct linger linger_opt = {1, 0};
      setsockopt(server_fd, SOL_SOCKET, SO_LINGER,
                 reinterpret_cast<const char*>(&linger_opt), sizeof(linger_opt));
      shutdown(server_fd, SHUT_RDWR);
      closeSocket(server_fd);
    }
    server_fd = INVALID_SOCKET_HANDLE;
    _status = wl_status_t::WL_DISCONNECTED;
  }
  WiFiClient accept() { return available_(); }
  WiFiClient available(uint8_t* = nullptr) { return available_(); }
  size_t write(uint8_t ch) override { return write(&ch, 1); }
  size_t write(const uint8_t* buf, size_t size) override {
    int rc = ::send(server_fd, reinterpret_cast<const char*>(buf), static_cast<int>(size), 0);
    return rc < 0 ? 0 : rc;
  }
  int status() { return _status; }
  void setNoDelay(bool nodelay) { _noDelay = nodelay; }
  bool getNoDelay() { return _noDelay; }
  bool hasClient() {
    if (server_fd == INVALID_SOCKET_HANDLE) return false;
    WSAPOLLFD pfd{};
    pfd.fd = server_fd;
    pfd.events = POLLRDNORM;
    return WSAPoll(&pfd, 1, 0) > 0 && (pfd.revents & POLLRDNORM);
  }
  using Print::write;

 protected:
  bool begin_(int port = 0) {
    if (port > 0) _port = port;
    _status = wl_status_t::WL_DISCONNECTED;
    ensureSocketRuntime();
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET_HANDLE) {
      _status = wl_status_t::WL_CONNECT_FAILED;
      return false;
    }
    int option = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&option), sizeof(option));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port);
    while (::bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr),
                  sizeof(server_addr)) < 0) {
      Logger.error("bind failed");
      delay(1000);
    }
    if (::listen(server_fd, 10) < 0) {
      _status = wl_status_t::WL_CONNECT_FAILED;
      return false;
    }
    _noDelay = false;
    active_servers().push_back(this);
    _status = wl_status_t::WL_CONNECTED;
    return true;
  }
  void setBlocking(bool flag) { is_blocking = flag; }

  EthernetClient available_() {
    struct sockaddr_in client_addr;
    SocketLength client_addr_len = sizeof(client_addr);
    SocketHandle client_fd;
    if (_status == wl_status_t::WL_CONNECT_FAILED) begin(_port);
    WSAPOLLFD pfd{};
    pfd.fd = server_fd;
    pfd.events = POLLRDNORM;
    int poll_rc = WSAPoll(&pfd, 1, 200);
    if (!is_blocking && (poll_rc <= 0 || !(pfd.revents & POLLRDNORM)))
      return EthernetClient(nullptr);
    if ((client_fd = ::accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr),
                              &client_addr_len)) == INVALID_SOCKET_HANDLE) {
      Logger.error("accept failed");
      return EthernetClient(nullptr);
    }
    setSocketNonBlocking(client_fd);
    return EthernetClient(std::make_shared<SocketImpl>(client_fd, &client_addr));
  }
};

}  // namespace arduino
