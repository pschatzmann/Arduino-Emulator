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
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

#include "Ethernet.h"
#include "SocketImpl.h"

namespace arduino {

#define SOCKET_IMPL_SEC "SocketImplSecure"

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
  size_t read(uint8_t* buffer, size_t len) {
    // size_t result = ::recv(sock, buffer, len, MSG_DONTWAIT );
    if (ssl == nullptr) {
      wolfSSL_set_fd(ssl, sock);
    }
    int result = ::wolfSSL_read(ssl, buffer, len);

    if (result < 0) {
      result = 0;
    }
    // 
    char lenStr[80];
    sprintf(lenStr, "%ld -> %d", len, result);
    Logger.debug(SOCKET_IMPL_SEC, "read->", lenStr);

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

/**
 * @brief NetworkClientSecure based on wolf ssl
 */
class NetworkClientSecure : public EthernetClient {
 public:
  NetworkClientSecure(int bufferSize = 256, long timeout = 2000)
      : EthernetClient(std::make_shared<SocketImplSecure>(), bufferSize, timeout) {}
  void setCACert(const char* cert) override { p_sock->setCACert(cert); }
  void setInsecure() override { p_sock->setInsecure(); }
};

}  // namespace arduino

#endif
