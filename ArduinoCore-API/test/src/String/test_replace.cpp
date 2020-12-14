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

TEST_CASE ("Testing String::replace(char, char) when string is empty", "[String-replace-01]")
{
  arduino::String str;
  str.replace('a', 'b');
  REQUIRE(str.length() == 0);
}

TEST_CASE ("Testing String::replace(char, char) when string contains elements != 'find'", "[String-replace-02]")
{
  arduino::String str("Hello Arduino!");
  str.replace('Z', '0');
  REQUIRE(strcmp(str.c_str(), "Hello Arduino!") == 0);
}

TEST_CASE ("Testing String::replace(char, char) when string contains elements = 'find'", "[String-replace-03]")
{
  arduino::String str("Hello Arduino!");
  str.replace('o', '0');
  str.replace('e', '3');
  str.replace('i', '1');
  REQUIRE(strcmp(str.c_str(), "H3ll0 Ardu1n0!") == 0);
}

TEST_CASE ("Testing String::replace(String, String) when string does not constain subtr 'find'", "[String-replace-04]")
{
  arduino::String str("Hello Arduino!");
  str.replace(arduino::String("Zulu"), arduino::String("11"));
  REQUIRE(strcmp(str.c_str(), "Hello Arduino!") == 0);
}

TEST_CASE ("Testing String::replace(String, String) when string constains subtr 'find'", "[String-replace-05]")
{
  arduino::String str("Hello Arduino!");
  str.replace(arduino::String("ll"), arduino::String("11"));
  REQUIRE(strcmp(str.c_str(), "He11o Arduino!") == 0);
}

TEST_CASE ("Testing String::replace(String, String) substr 'find' larger than 'replace'", "[String-replace-06]")
{
  arduino::String str("Hello Arduino!");
  str.replace(arduino::String("llo"), arduino::String("11"));
  REQUIRE(strcmp(str.c_str(), "He11 Arduino!") == 0);
}

TEST_CASE ("Testing String::replace(String, String) substr 'find' smaller than 'replace'", "[String-replace-07]")
{
  arduino::String str("Hello Arduino!");
  str.replace(arduino::String("ll"), arduino::String("111"));
  REQUIRE(strcmp(str.c_str(), "He111o Arduino!") == 0);
}
