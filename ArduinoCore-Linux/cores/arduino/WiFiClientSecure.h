#pragma once
#if defined(USE_HTTPS)
#include "NetworkClientSecure.h"

namespace arduino {

/**
 * @brief Alias for secure WiFi client using TLS/SSL.
 *
 * Use this type for secure network connections (HTTPS, TLS) on platforms supporting NetworkClientSecure.
 */
using WiFiClientSecure = NetworkClientSecure;
  
} // namespace arduino
#else

#include "WiFiClient.h"

#endif
