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

TEST_CASE ("Testing String::concat(const String &)", "[String-concat-01]")
{
  arduino::String str1("Hello "), str2("Arduino!");
  REQUIRE(str1.concat(str2) == 1);
  REQUIRE(strcmp(str1.c_str(), "Hello Arduino!") == 0);
}

TEST_CASE ("Testing String::concat(const char *)", "[String-concat-02]")
{
  arduino::String str("Hello ");
  REQUIRE(str.concat("Arduino!") == 1);
  REQUIRE(strcmp(str.c_str(), "Hello Arduino!") == 0);
}

TEST_CASE ("Testing String::concat(char)", "[String-concat-03]")
{
  arduino::String str("Hello ");
  char const c = 'A';
  REQUIRE(str.concat(c) == 1);
  REQUIRE(strcmp(str.c_str(), "Hello A") == 0);
}

TEST_CASE ("Testing String::concat(unsigned char)", "[String-concat-04]")
{
  arduino::String str("Hello ");
  unsigned char const c = 'A';
  REQUIRE(str.concat(c) == 1);
  REQUIRE(strcmp(str.c_str(), "Hello 65") == 0); /* ASCII['A'] = 65 */
}

TEST_CASE ("Testing String::concat(int)", "[String-concat-05]")
{
  arduino::String str("Hello ");
  int const num = -1;
  REQUIRE(str.concat(num) == 1);
  REQUIRE(strcmp(str.c_str(), "Hello -1") == 0);
}

TEST_CASE ("Testing String::concat(unsigned int)", "[String-concat-06]")
{
  arduino::String str("Hello ");
  unsigned int const num = 1;
  REQUIRE(str.concat(num) == 1);
  REQUIRE(strcmp(str.c_str(), "Hello 1") == 0);
}

TEST_CASE ("Testing String::concat(long)", "[String-concat-07]")
{
  arduino::String str("Hello ");
  long const num = -1;
  REQUIRE(str.concat(num) == 1);
  REQUIRE(strcmp(str.c_str(), "Hello -1") == 0);
}

TEST_CASE ("Testing String::concat(unsigned long)", "[String-concat-08]")
{
  arduino::String str("Hello ");
  unsigned long const num = 1;
  REQUIRE(str.concat(num) == 1);
  REQUIRE(strcmp(str.c_str(), "Hello 1") == 0);
}

TEST_CASE ("Testing String::concat(float)", "[String-concat-09]")
{
  arduino::String str("Hello ");
  float const num = 1.234f;
  REQUIRE(str.concat(num) == 1);
  REQUIRE(strcmp(str.c_str(), "Hello 1.23") == 0);
}

TEST_CASE ("Testing String::concat(double)", "[String-concat-10]")
{
  arduino::String str("Hello ");
  double const num = 5.678;
  REQUIRE(str.concat(num) == 1);
  REQUIRE(strcmp(str.c_str(), "Hello 5.68") == 0);
}

TEST_CASE ("Testing String::concat(const __FlashStringHelper *)", "[String-concat-11]")
{
#undef F
#define F(string_literal) (reinterpret_cast<const arduino::__FlashStringHelper *>(PSTR(string_literal)))
  arduino::String str1("Hello");
  REQUIRE(str1.concat(F(" Arduino")) == 1);
  REQUIRE(strcmp(str1.c_str(), "Hello Arduino") == 0);
}