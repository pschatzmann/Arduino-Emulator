#pragma once

#include <unistd.h>
#include "Server.h"
#include "Ethernet.h"
#include <poll.h>

namespace arduino {

/**
 * A minimal ethernet server
*/
class EthernetServer : public Server {
 private:
  uint16_t _port;
  int server_fd;
  struct sockaddr_in server_addr;
  int _status = wl_status_t::WL_DISCONNECTED;
  bool is_blocking = false;

 public:
  EthernetServer(int port = 80) { _port = port; }
  void begin() {  begin(0); }
  void begin(int port) { begin_(port); }
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

    // create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      // error("socket failed");
      _status = wl_status_t::WL_CONNECT_FAILED;
      return false;
    }

    // config socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port);

    // bind socket to port
    if (::bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
        0) {
      // error("bind failed");
      _status = wl_status_t::WL_CONNECT_FAILED;
      return false;
    }

    // listen for connections 
    if (::listen(server_fd, 10) < 0) {
      // error("listen failed");
      _status = wl_status_t::WL_CONNECT_FAILED;
      return false;
    }

    return true;
  }

  void setBlocking(bool flag){
    is_blocking = flag;
  }

  EthernetClient available_() {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;

    struct pollfd pfd;
    pfd.fd = server_fd;
    pfd.events = POLLIN;
    int poll_rc = ::poll(&pfd, 1, 200);

    // non blocking check if we have any request to accept
    if (!is_blocking){
      if (poll_rc <= 0 || !pfd.revents & POLLIN){
        EthernetClient result;
        return result;
      }
    }

    // accept client connection (blocking call)
    if ((client_fd = ::accept(server_fd, (struct sockaddr*)&client_addr,
                            &client_addr_len)) < 0) {
      // perror("accept failed");
      EthernetClient result;
      return result;
    }
    SocketImpl sock_impl(client_fd, (struct sockaddr_in*) &client_addr);
    EthernetClient result{sock_impl};
    return result;
  }
};

}  // namespace arduino
