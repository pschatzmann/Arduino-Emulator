#pragma once

#include "Ethernet.h"

namespace arduino {

using WiFiClient = EthernetClient;

#if !defined(USE_HTTPS)
using WiFiClientSecure = EthernetClient;
#endif
}
