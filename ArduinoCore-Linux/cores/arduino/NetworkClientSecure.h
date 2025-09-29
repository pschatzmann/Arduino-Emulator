#pragma once
#if defined(USE_HTTPS)
#include <errno.h>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

#include "Ethernet.h"
#include "SocketImpl.h"

namespace arduino {

#define SOCKET_IMPL_SEC "SocketImplSecure"

static int wolf_ssl_counter = 0;
static WOLFSSL_CTX* wolf_ctx = nullptr;

/**
 * @brief SSL Socket using wolf ssl
 */
class SocketImplSecure : public SocketImpl {
 public:
  SocketImplSecure() {
    if (wolf_ssl_counter++ == 0 || wolf_ctx == nullptr) {
      wolfSSL_Init();
      if ((wolf_ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method())) == NULL) {
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
  size_t read(uint8_t* buffer, size_t len) {
    // size_t result = ::recv(sock, buffer, len, MSG_DONTWAIT );
    if (ssl == nullptr) {
      wolfSSL_set_fd(ssl, sock);
    }
    int result = ::wolfSSL_read(ssl, buffer, len);

    char lenStr[80];
    sprintf(lenStr, "%ld -> %d", len, result);
    if (result < 0) {
      Logger.error(SOCKET_IMPL_SEC, "read->", lenStr);
      return 0;
    } else {
      Logger.debug(SOCKET_IMPL_SEC, "read->", lenStr);
    }
    return result;
  }

  int connect(const char* address, uint16_t port) override {
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

    // Connect to server
    if (::connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      Logger.error(SOCKET_IMPL_SEC, "Connection failed");
      ::close(sock);
      sock = -1;
      return -1;
    }

    // Set SSL file descriptor
    wolfSSL_set_fd(ssl, sock);

    // Perform SSL handshake
    if (wolfSSL_connect(ssl) != SSL_SUCCESS) {
      Logger.error(SOCKET_IMPL_SEC, "SSL handshake failed");
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
    if (wolf_ctx == nullptr) return;
    // Disable certificate verification
    wolfSSL_CTX_set_verify(wolf_ctx, SSL_VERIFY_NONE, nullptr);
  }

 protected:
  WOLFSSL* ssl = nullptr;
};

/**
 * @brief NetworkClientSecure based on wolf ssl
 */
class NetworkClientSecure : public EthernetClient {
 public:
  NetworkClientSecure(int bufferSize = 256, long timeout = 2000)
      : EthernetClient(new SocketImplSecure(), bufferSize, timeout) {}
  void setCACert(const char* cert) override { p_sock->setCACert(cert); }
  void setInsecure() override { p_sock->setInsecure(); }
};

}  // namespace arduino

#endif
