#pragma once
#include "EthernetServer.h"

namespace arduino {

/**
 * @brief Alias for EthernetServer to provide WiFiServer compatibility.
 */
using WiFiServer = EthernetServer;

}  // namespace arduino