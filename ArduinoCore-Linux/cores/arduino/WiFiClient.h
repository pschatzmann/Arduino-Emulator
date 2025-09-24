#pragma once

#include "Ethernet.h"

namespace arduino {

/**
 * @brief Alias for EthernetClient to provide WiFiClient compatibility.
 */
using WiFiClient = EthernetClient;

#if !defined(USE_HTTPS)
/**
 * @brief Alias for EthernetClient to provide WiFiClientSecure compatibility when HTTPS is not used.
 */
using WiFiClientSecure = EthernetClient;
#endif
}
