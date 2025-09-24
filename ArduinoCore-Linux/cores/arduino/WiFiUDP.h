#pragma once
#include "EthernetUDP.h"

namespace arduino {

/**
 * @brief Alias for EthernetUDP to provide WiFiUDP compatibility.
 */
using WiFiUDP = EthernetUDP;

}