
add_subdirectory("hallo-world")
add_subdirectory("blink")
add_subdirectory("i2c")
add_subdirectory("spi")
add_subdirectory("serial2")
add_subdirectory("using-arduino-library")
add_subdirectory("pwm")

if(USE_HTTPS)
    add_subdirectory("wifi-secure")
endif(USE_HTTPS)

