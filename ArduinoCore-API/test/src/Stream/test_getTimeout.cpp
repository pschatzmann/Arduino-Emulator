/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <StreamMock.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Verifying if default timeout is returned correctly", "[Stream-getTimeout-01]")
{
  StreamMock mock;
  REQUIRE(mock.getTimeout() == 1000);
}
