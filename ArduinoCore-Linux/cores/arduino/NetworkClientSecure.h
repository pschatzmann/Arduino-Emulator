/*
	NetworkClientSecure.h
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
#if defined(USE_HTTPS)
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>
#include <ctime>
#include <memory>
#include <string>

#include "Ethernet.h"
#include "SocketImpl.h"

// Two TLS backends: TinyTLS (default) or wolfSSL (USE_WOLFSSL).
#if defined(USE_WOLFSSL)
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#else
#include "TinyTLS.h"
#endif

namespace arduino {

#define SOCKET_IMPL_SEC "SocketImplSecure"

#if defined(USE_WOLFSSL)

static int wolf_ssl_counter = 0;
static WOLFSSL_CTX* wolf_ctx = nullptr;

/**
 * @brief SSL Socket using wolf ssl. For error codes see
 * https://wolfssl.jp/docs-3/wolfssl-manual/appendix-c
 */
class SocketImplSecure : public SocketImpl {
 public:
  SocketImplSecure() {
    if (wolf_ssl_counter++ == 0 || wolf_ctx == nullptr) {
      wolfSSL_Init();
      if ((wolf_ctx = wolfSSL_CTX_new(wolfTLS_client_method())) == NULL) {
        Logger.error(SOCKET_IMPL_SEC, "wolfSSL_CTX_new error.");
      }
    }
    if ((ssl = wolfSSL_new(wolf_ctx)) == NULL) {
      Logger.error(SOCKET_IMPL_SEC, "wolfSSL_new error.");
    }
  }

  ~SocketImplSecure() {
    if (ssl) {
      wolfSSL_free(ssl);
      ssl = nullptr;
    }
    if (--wolf_ssl_counter == 0 && wolf_ctx) {
      wolfSSL_CTX_free(wolf_ctx);
      wolf_ctx = nullptr;
      wolfSSL_Cleanup();
    }
  }
  // direct read
  int read(uint8_t* buffer, size_t len) override {
    // size_t result = ::recv(sock, buffer, len, MSG_DONTWAIT );
    if (ssl == nullptr) {
      wolfSSL_set_fd(ssl, sock);
    }
    int result = ::wolfSSL_read(ssl, buffer, len);

    if (result < 0) {
      int error = wolfSSL_get_error(ssl, result);
      if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
        return 0;
      }

      is_connected = false;
      return -1;
    }

    if (result == 0) {
      is_connected = false;
      return -1;
    }

    // 
    char lenStr[80];
    sprintf(lenStr, "%ld -> %d", len, result);
    Logger.debug(SOCKET_IMPL_SEC, "read->", lenStr);

    return result;
  }

  int connect(const char* address, uint16_t port) override {
    return connect(address, port, -1);
  }

  // opens a connection with a timeout in milliseconds for the TCP handshake.
  // The TLS handshake below still runs to completion without its own
  // timeout, matching the previous (2-arg) behavior.
  int connect(const char* address, uint16_t port, int32_t timeout_ms) override {
    // Create socket
    sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
      Logger.error(SOCKET_IMPL_SEC, "Socket creation failed");
      return -1;
    }

    // Setup server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
      Logger.error(SOCKET_IMPL_SEC, "Invalid address", address);
      ::close(sock);
      sock = -1;
      return -1;
    }

    // Connect to server, honoring timeout_ms when >= 0
    if (timeout_ms < 0) {
      if (::connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        Logger.error(SOCKET_IMPL_SEC, "Connection failed");
        ::close(sock);
        sock = -1;
        return -1;
      }
    } else {
      setSocketNonBlocking(sock, true);

      int result = ::connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
      if (result < 0 && errno != EINPROGRESS && errno != EWOULDBLOCK) {
        Logger.error(SOCKET_IMPL_SEC, "Connection failed");
        setSocketNonBlocking(sock, false);
        ::close(sock);
        sock = -1;
        return -1;
      }

      if (result < 0) {
        fd_set writefds, errorfds;
        FD_ZERO(&writefds);
        FD_SET(sock, &writefds);
        FD_ZERO(&errorfds);
        FD_SET(sock, &errorfds);
        timeval timeout{.tv_sec = timeout_ms / 1000,
                         .tv_usec = (timeout_ms % 1000) * 1000};

        result = select(sock + 1, nullptr, &writefds, &errorfds, &timeout);
        int error_code = 0;
        socklen_t error_len = sizeof(error_code);
        if (result <= 0 ||
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &error_code, &error_len) != 0 ||
            error_code != 0) {
          Logger.error(SOCKET_IMPL_SEC,
                        result == 0 ? "Connection timeout" : "Connection failed");
          setSocketNonBlocking(sock, false);
          ::close(sock);
          sock = -1;
          return -1;
        }
      }

      setSocketNonBlocking(sock, false);
    }

    // Reset the TLS session for this connection - reusing ssl as-is
    // across a stop()/connect() cycle (this object's owner, e.g.
    // URLStream's own client_secure, is typically kept and reconnected
    // rather than recreated) leaves it in stale post-handshake state, so
    // a second connect() would send a request the server never replies
    // to instead of doing a fresh handshake.
    if (ssl != nullptr) {
      wolfSSL_free(ssl);
    }
    ssl = wolfSSL_new(wolf_ctx);
    if (is_insecure && ssl != nullptr) {
      wolfSSL_set_verify(ssl, SSL_VERIFY_NONE, nullptr);
    }

    // Set SSL file descriptor
    wolfSSL_set_fd(ssl, sock);

    // Set SNI (Server Name Indication) if needed
    wolfSSL_UseSNI(ssl, WOLFSSL_SNI_HOST_NAME, address, strlen(address));

    // Perform SSL handshake
    int rc = wolfSSL_connect(ssl);
    if (rc != SSL_SUCCESS) {
      int err = wolfSSL_get_error(ssl, rc);
      const char* errStr = wolfSSL_ERR_reason_error_string(err);
      char msg[160];
      snprintf(msg, sizeof(msg),
               "SSL handshake failed, error code: %d reason: %s", err,
               errStr ? errStr : "unknown");
      Logger.error(SOCKET_IMPL_SEC, msg);
      ::close(sock);
      sock = -1;
      return -1;
    }

    is_connected = true;
    return 1;
  }

  // send the data via the socket - returns the number of characters written or
  // -1=>Error
  size_t write(const uint8_t* str, size_t len) {
    Logger.debug(SOCKET_IMPL_SEC, "write");
    if (ssl == nullptr) {
      wolfSSL_set_fd(ssl, sock);
    }
    // return ::send(sock , str , len , 0 );
    return ::wolfSSL_write(ssl, str, len);
  }

  void setCACert(const char* cert) override {
    if (wolf_ctx == nullptr) return;
    // Load CA certificate from a PEM string
    int ret =
        wolfSSL_CTX_load_verify_buffer(wolf_ctx, (const unsigned char*)cert,
                                       strlen(cert), WOLFSSL_FILETYPE_PEM);
    if (ret != SSL_SUCCESS) {
      Logger.error(SOCKET_IMPL_SEC, "Failed to load CA certificate");
    }
  }

  void setInsecure() {
    is_insecure = true;
    if (wolf_ctx == nullptr) return;
    // Disable certificate verification on context
    wolfSSL_CTX_set_verify(wolf_ctx, SSL_VERIFY_NONE, nullptr);
    // Also disable on SSL object if already created
    if (ssl) wolfSSL_set_verify(ssl, SSL_VERIFY_NONE, nullptr);
  }

 protected:
  WOLFSSL* ssl = nullptr;
  bool is_insecure = false;
};

#else  // TinyTLS (default)

namespace tls_detail {
// TinyTLS callbacks: monotonic clock (ms), wall clock (s) and idle wait
static uint32_t clockMs() { return static_cast<uint32_t>(millis()); }
static int64_t wallTime() { return static_cast<int64_t>(::time(nullptr)); }
static void idleWait() { delay(1); }
}  // namespace tls_detail

/**
 * @brief SSL Socket using TinyTLS (TLS 1.3 client). The TLS session runs over
 * the plain TCP connection of this socket.
 */
class SocketImplSecure : public SocketImpl {
 public:
  static constexpr uint32_t DEFAULT_HANDSHAKE_TIMEOUT_MS = 15000;

  SocketImplSecure() {
    tls_.attach(transport_);
    tls_.setClock(tls_detail::clockMs);
    tls_.setTimeSource(tls_detail::wallTime);
    tls_.setIdleCallback(tls_detail::idleWait);
  }

  ~SocketImplSecure() { tls_.stop(); }

  // opens a TCP connection and performs the TLS handshake
  int connect(const char* address, uint16_t port) override {
    return connect(address, port, -1);
  }

  int connect(const char* address, uint16_t port, int32_t timeout_ms) override {
    tcp_timeout_ms_ = timeout_ms;
    tls_.setTimeout(timeout_ms >= 0 ? static_cast<uint32_t>(timeout_ms)
                                    : DEFAULT_HANDSHAKE_TIMEOUT_MS);
    // use the host name (not the resolved IP) for SNI and certificate checks
    const char* name = hostname_.empty() ? address : hostname_.c_str();
    if (!tls_.connect(name, port)) {
      char msg[160];
      snprintf(msg, sizeof(msg), "TLS connect to %s failed: %s", name,
               tls_.errorText());
      Logger.error(SOCKET_IMPL_SEC, msg);
      return -1;
    }
    return 1;
  }

  // name used for SNI and certificate verification
  void setHostname(const char* name) override {
    hostname_ = name ? name : "";
  }

  uint8_t connected() override { return tls_.connected(); }

  // number of decrypted bytes that can be read without blocking
  size_t available() override {
    int n = tls_.available();
    return (n > 0 ? static_cast<size_t>(n) : 0) + (has_peek_ ? 1 : 0);
  }

  // returns the decrypted bytes; 0 if none is available yet, -1 if closed
  int read(uint8_t* buffer, size_t len) override {
    if (len == 0) return 0;
    if (has_peek_) {
      buffer[0] = peek_byte_;
      has_peek_ = false;
      return 1;
    }
    int result = tls_.read(buffer, len);
    if (result > 0) return result;
    return tls_.connected() ? 0 : -1;
  }

  int peek() override {
    if (!has_peek_) {
      if (read(&peek_byte_, 1) != 1) return -1;
      has_peek_ = true;
    }
    return peek_byte_;
  }

  size_t write(const uint8_t* str, size_t len) override {
    return tls_.write(str, len);
  }

  void close() override { tls_.stop(); }

  void setCACert(const char* cert) override {
    if (cert == nullptr) return;
    if (!roots_) roots_.reset(new tinytls::CertificateStore());
    tinytls::Error rc = roots_->addPem(reinterpret_cast<const uint8_t*>(cert),
                                       strlen(cert));
    if (rc != tinytls::Error::Ok) {
      Logger.error(SOCKET_IMPL_SEC, "Failed to load CA certificate");
    }
    tls_.setTrustStore(roots_.get());
  }

  // disables certificate verification (encryption only)
  void setInsecure() override {
    tls_.setVerifyMode(tinytls::VerifyMode::None);
  }

 private:
  // Plain TCP operations used by the TLS session. They use the non-virtual
  // SocketImpl methods, so base class callbacks cannot re-enter the TLS session.
  class SocketTransport : public tinytls::Transport {
   public:
    explicit SocketTransport(SocketImplSecure& owner) : owner_(owner) {}

    bool connect(const char* host, uint16_t port) override {
      return owner_.SocketImpl::connect(host, port, owner_.tcp_timeout_ms_) > 0;
    }

    size_t write(const uint8_t* buf, size_t len) override {
      if (owner_.sock < 0) return 0;
      ssize_t n = ::send(owner_.sock, buf, len, MSG_NOSIGNAL);
      if (n >= 0) return static_cast<size_t>(n);
      if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
      owner_.SocketImpl::close();
      return 0;
    }

    int read(uint8_t* buf, size_t len) override {
      if (owner_.sock < 0) return -1;
      ssize_t n = ::recv(owner_.sock, buf, len, MSG_DONTWAIT);
      if (n > 0) return static_cast<int>(n);
      if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return -1;
      owner_.SocketImpl::close();  // peer closed or error
      return -1;
    }

    int available() override {
      if (owner_.sock < 0) return 0;
      int bytes = 0;
      if (ioctl(owner_.sock, FIONREAD, &bytes) != 0) return 0;
      return bytes > 0 ? bytes : 0;
    }

    bool connected() override {
      if (owner_.sock < 0) return false;
      char c;
      ssize_t n = ::recv(owner_.sock, &c, 1, MSG_PEEK | MSG_DONTWAIT);
      if (n > 0 || (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))) return true;
      owner_.SocketImpl::close();
      return false;
    }

    void stop() override { owner_.SocketImpl::close(); }

   private:
    SocketImplSecure& owner_;
  };

  std::unique_ptr<tinytls::CertificateStore> roots_;  // declared before tls_
  SocketTransport transport_{*this};                  // declared before tls_
  TinyTLS tls_;
  std::string hostname_;
  int32_t tcp_timeout_ms_ = -1;
  bool has_peek_ = false;
  uint8_t peek_byte_ = 0;
};

#endif  // USE_WOLFSSL

/**
 * @brief NetworkClientSecure based on TinyTLS (default) or wolf ssl
 */
class NetworkClientSecure : public EthernetClient {
 public:
  NetworkClientSecure(int bufferSize = 256, long timeout = 2000)
      : EthernetClient(std::make_shared<SocketImplSecure>(), bufferSize, timeout) {}

  using EthernetClient::connect;

  // remember the host name so that TLS uses it for SNI and certificate checks
  int connect(const char* host, uint16_t port) override {
    p_sock->setHostname(host);
    return EthernetClient::connect(host, port);
  }

  int connect(const char* host, uint16_t port, int32_t timeout_ms) {
    p_sock->setHostname(host);
    return EthernetClient::connect(host, port, timeout_ms);
  }

  void setCACert(const char* cert) override { p_sock->setCACert(cert); }
  void setInsecure() override { p_sock->setInsecure(); }
};

}  // namespace arduino

#endif
