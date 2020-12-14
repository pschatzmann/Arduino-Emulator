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

TEST_CASE ("Testing String::trim with space at the beginning", "[String-trim-01]")
{
  arduino::String str("  hello");
  str.trim();
  REQUIRE(strcmp(str.c_str(), "hello") == 0);
}

TEST_CASE ("Testing String::trim with space at the end", "[String-trim-02]")
{
  arduino::String str("hello  ");
  str.trim();
  REQUIRE(strcmp(str.c_str(), "hello") == 0);
}

TEST_CASE ("Testing String::trim with space at both beginng and end", "[String-trim-03]")
{
  arduino::String str("  hello  ");
  str.trim();
  REQUIRE(strcmp(str.c_str(), "hello") == 0);
}

TEST_CASE ("Testing String::trim with space in the middle", "[String-trim-04]")
{
  arduino::String str("Hello Arduino!");
  str.trim();
  REQUIRE(strcmp(str.c_str(), "Hello Arduino!") == 0);
}
