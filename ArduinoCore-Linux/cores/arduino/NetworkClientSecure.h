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

/**
 * @brief SSL Socket using wolf ssl
 */
class SocketImplSecure : public SocketImpl {
 public:
  SocketImplSecure() {
    if (wolf_ssl_counter++ == 0 || ctx == nullptr) {
      wolfSSL_Init();
      if ((ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method())) == NULL) {
        Logger.error(SOCKET_IMPL_SEC, "wolfSSL_CTX_new error.");
      }
    }
    if ((ssl = wolfSSL_new(ctx)) == NULL) {
      Logger.error(SOCKET_IMPL_SEC, "wolfSSL_new error.");
    }
  }

  ~SocketImplSecure() {
    if (ssl) {
      wolfSSL_free(ssl);
      ssl = nullptr;
    }
    if (--wolf_ssl_counter == 0 && ctx) {
      wolfSSL_CTX_free(ctx);
      ctx = nullptr;
      wolfSSL_Cleanup();
    }
  }
  // direct read
  size_t read(uint8_t* buffer, size_t len) {
    // size_t result = ::recv(sock, buffer, len, MSG_DONTWAIT );
    if (ssl == nullptr) {
      wolfSSL_set_fd(ssl, sock);
    }
    size_t result = ::wolfSSL_read(ssl, buffer, len);
    char lenStr[80];
    sprintf(lenStr, "%ld -> %ld", len, result);
    Logger.debug(SOCKET_IMPL_SEC, "read->", lenStr);
    return result;
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
    if (ctx == nullptr) return;
    // Load CA certificate from a PEM string
    int ret = wolfSSL_CTX_load_verify_buffer(
        ctx, (const unsigned char*)cert, strlen(cert), WOLFSSL_FILETYPE_PEM);
    if (ret != SSL_SUCCESS) {
      Logger.error(SOCKET_IMPL_SEC, "Failed to load CA certificate");
    }
  }

  void setInsecure() {
    if (ctx == nullptr) return;
    // Disable certificate verification
    wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
  }

 protected:
  WOLFSSL_CTX* ctx = nullptr;
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
