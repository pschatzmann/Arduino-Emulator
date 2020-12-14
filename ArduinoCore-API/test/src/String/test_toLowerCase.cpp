/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <String.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Testing String::toLowerCase", "[String-toLowerCase-01]")
{
  arduino::String str("HELLO ARDUINO");
  str.toLowerCase();
  REQUIRE(strcmp(str.c_str(), "hello arduino") == 0);
}
