/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <IPAddress.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Testing IPAddress::operator uint32_t() const", "[IPAddress-Operator-()-01]")
{
  arduino::IPAddress ip(129,168,1,2);
  uint32_t const val_expected = ip;
  uint32_t const val_actual = (129 | (168 << 8) | (1 << 16) | (2 << 24));
  REQUIRE(val_expected == val_actual);
}
