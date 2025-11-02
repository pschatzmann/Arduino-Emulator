
/*
  EthernetServer.h 
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
#pragma once

#include <poll.h>
#include <unistd.h>

#include "Ethernet.h"
#include "api/Server.h"
#include "SignalHandler.h"

namespace arduino {

/**
 * A minimal ethernet server
 */

class EthernetServer : public Server {
 private:
  uint16_t _port;
  int server_fd = 0;
  struct sockaddr_in server_addr;
  int _status = wl_status_t::WL_DISCONNECTED;
  bool is_blocking = false;
  static std::vector<EthernetServer*>& active_servers() {
    static std::vector<EthernetServer*> servers;
    return servers;
  }
  static void cleanupAll(int sig) {
    for (auto* server : active_servers()) {
      if (server && server->server_fd > 0) {
        shutdown(server->server_fd, SHUT_RDWR);
        close(server->server_fd);
        server->server_fd = 0;
      }
    }
  }

 public:
  EthernetServer(int port = 80) {
    _port = port;
    // Register signal handler only once
    static bool signal_registered = false;
    if (!signal_registered) {
      SignalHandler::registerHandler(SIGINT, cleanupAll);
      SignalHandler::registerHandler(SIGTERM, cleanupAll);
      signal_registered = true;
    }
  }

  ~EthernetServer() {
    stop();
    // Remove from active servers list
    auto& servers = active_servers();
    auto it = std::find(servers.begin(), servers.end(), this);
    if (it != servers.end()) {
      servers.erase(it);
    }
  }
  void begin() { begin(0); }
  void begin(int port) { begin_(port); }
  void stop() {
    if (server_fd > 0) {
      // Set SO_LINGER to force immediate close
      struct linger linger_opt = {1, 0};
      setsockopt(server_fd, SOL_SOCKET, SO_LINGER, &linger_opt,
                 sizeof(linger_opt));

      shutdown(server_fd, SHUT_RDWR);
      close(server_fd);
    }
    server_fd = 0;
    _status = wl_status_t::WL_DISCONNECTED;
  }
  WiFiClient accept() { return available_(); }
  WiFiClient available(uint8_t* status = NULL) { return available_(); }
  virtual size_t write(uint8_t ch) { return write(&ch, 1); }
  virtual size_t write(const uint8_t* buf, size_t size) {
    int rc = ::write(server_fd, buf, size);
    if (rc < 0) {
      rc = 0;
    }
    return rc;
  }
  int status() { return _status; }

  using Print::write;

 protected:
  bool begin_(int port = 0) {
    if (port > 0) _port = port;
    _status = wl_status_t::WL_DISCONNECTED;

    // create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      // error("socket failed");
      _status = wl_status_t::WL_CONNECT_FAILED;
      return false;
    }

    // Reuse address after restart
    int iSetOption = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption,
               sizeof(iSetOption));

    // Set SO_REUSEPORT for better port reuse
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, (char*)&iSetOption,
               sizeof(iSetOption));

    // config socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port);

    // bind socket to port
    while (::bind(server_fd, (struct sockaddr*)&server_addr,
                  sizeof(server_addr)) < 0) {
      // error("bind failed");
      //_status = wl_status_t::WL_CONNECT_FAILED;
      Logger.error("bind failed");
      // return false;
      delay(1000);
    }

    // listen for connections
    if (::listen(server_fd, 10) < 0) {
      // error("listen failed");
      _status = wl_status_t::WL_CONNECT_FAILED;
      Logger.error("listen failed");
      return false;
    }

    // Add to active servers list for signal handling
    active_servers().push_back(this);
    _status = wl_status_t::WL_CONNECTED;
    return true;
  }

  void setBlocking(bool flag) { is_blocking = flag; }

  EthernetClient available_() {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;

    if (_status == wl_status_t::WL_CONNECT_FAILED) {
      begin(_port);
    }

    struct pollfd pfd;
    pfd.fd = server_fd;
    pfd.events = POLLIN;
    int poll_rc = ::poll(&pfd, 1, 200);

    // non blocking check if we have any request to accept
    if (!is_blocking) {
      if (poll_rc <= 0 || !(pfd.revents & POLLIN)) {
        EthernetClient result;
        return result;
      }
    }

    // accept client connection (blocking call)
    if ((client_fd = ::accept(server_fd, (struct sockaddr*)&client_addr,
                              &client_addr_len)) < 0) {
      EthernetClient result;
      Logger.error("accept failed");
      return result;
    }
    std::shared_ptr<SocketImpl> sock_impl = std::make_shared<SocketImpl>(client_fd, (struct sockaddr_in*)&client_addr);
    EthernetClient result{sock_impl};
    return result;
  }
};

}  // namespace arduino
