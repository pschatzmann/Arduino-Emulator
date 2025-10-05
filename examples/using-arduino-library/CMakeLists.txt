
# Add the arduino-SAM library
arduino_library(sam "https://github.com/pschatzmann/arduino-SAM")

# Use the arduino_sketch function to build the example with additional libraries
arduino_sketch(using-arduino-library using-arduino-library.ino LIBRARIES sam)

