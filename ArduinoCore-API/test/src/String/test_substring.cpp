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

TEST_CASE ("Testing String::substring(unsigned int, unsigned int)", "[String-substring-01]")
{
  WHEN ("left higher than len")
  {
    arduino::String str("Hello");
    str.substring(7,9);
  }

  WHEN ("right higher than len")
  {
    arduino::String str1("Hello");
    arduino::String str2("ello");
    REQUIRE(str2.compareTo(str1.substring(1,9)) == 0);
  }

  WHEN ("left higher than right")
  {
    arduino::String str1("Hello");
    arduino::String str2("ello");
    REQUIRE(str2.compareTo(str1.substring(9,1)) == 0);
  }
}