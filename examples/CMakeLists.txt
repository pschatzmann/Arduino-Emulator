
add_subdirectory("hallo-world")
add_subdirectory("blink")
add_subdirectory("i2c")
add_subdirectory("spi")
add_subdirectory("serial2")

if(USE_HTTPS)
    add_subdirectory("wifi-secure")
endif(USE_HTTPS)

if (USE_RPI)
    add_subdirectory("raspberry-pi")
endif(USE_RPI)
