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

TEST_CASE ("Verifying if calling 'setTimeout' is indeed modifying the timeout", "[Stream-setTimeout-01]")
{
  StreamMock mock;

  mock.setTimeout(100);

  REQUIRE(mock.getTimeout() == 100);
}
