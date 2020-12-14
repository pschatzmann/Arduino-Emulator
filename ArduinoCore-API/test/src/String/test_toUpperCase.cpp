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

TEST_CASE ("Testing String::toUpperCase", "[String-toUpperCase-01]")
{
  arduino::String str("hello arduino");
  str.toUpperCase();
  REQUIRE(strcmp(str.c_str(), "HELLO ARDUINO") == 0);
}
