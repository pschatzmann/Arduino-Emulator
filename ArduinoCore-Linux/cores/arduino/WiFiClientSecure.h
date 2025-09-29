#pragma once
#if defined(USE_HTTPS)
#include <wolfssl/ssl.h>
#include "Ethernet.h"

namespace arduino {
  
#define SOCKET_IMPL_SEC "SocketImplSecure"

/**
 * @brief SSL Socket using wolf ssl
 * 
 */
class SocketImplSecure : public SocketImpl {
public:
  SocketImplSecure() {
    wolfSSL_Init();
    if ((ctx = wolfSSL_CTX_new(wolfTLSv1_client_method())) == NULL) {
      Logger.error(SOCKET_IMPL_SEC, "wolfSSL_CTX_new error.");
    }
    if ((ssl = wolfSSL_new(ctx)) == NULL) {
      Logger.error(SOCKET_IMPL_SEC, "wolfSSL_new error.");
    }
  }
  // direct read
  size_t read(uint8_t *buffer, size_t len) {
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
  size_t write(const uint8_t *str, size_t len) {
    Logger.debug(SOCKET_IMPL_SEC, "write");
    if (ssl == nullptr) {
      wolfSSL_set_fd(ssl, sock);
    }
    // return ::send(sock , str , len , 0 );
    return ::wolfSSL_write(ssl, str, len);
  }

protected:
  WOLFSSL_CTX *ctx = nullptr;
  WOLFSSL *ssl = nullptr;
};

/**
 * @brief WiFiClientSecure based on wolf ssl 
 */
class WiFiClientSecure : public EthernetClient {
public:
  WiFiClientSecure(int bufferSize = 256, long timeout = 2000)
      : EthernetClient(SocketImplSecure { }, bufferSize, timeout) {}
};

} // namespace arduino

#endif
