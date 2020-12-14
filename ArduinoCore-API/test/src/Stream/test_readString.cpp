/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <MillisFake.h>
#include <StreamMock.h>

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE ("Testing 'readString' with data available within the stream", "[Stream-readString-01]")
{
  StreamMock mock;
  mock.setTimeout(10);
  millis_autoOn();
  mock << "This is test stream content";

  REQUIRE(mock.readString() == arduino::String("This is test stream content"));
}
