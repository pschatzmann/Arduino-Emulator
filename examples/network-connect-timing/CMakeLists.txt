arduino_sketch(network-connect-timing network-connect-timing.ino)

if(APPLE)
	target_compile_options(network-connect-timing PRIVATE -Wno-deprecated-declarations)
endif()