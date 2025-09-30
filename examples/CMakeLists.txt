
add_subdirectory("hallo-world")

if(USE_HTTPS)
    add_subdirectory("wifi-secure")
endif(USE_HTTPS)

if (USE_RPI)
    add_subdirectory("raspberry-pi")
endif(USE_RPI)
