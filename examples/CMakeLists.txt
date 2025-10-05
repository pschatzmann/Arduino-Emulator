
add_subdirectory("hallo-world")
add_subdirectory("blink")
add_subdirectory("i2c")
add_subdirectory("spi")
add_subdirectory("serial2")
add_subdirectory("using-arduino-library")
add_subdirectory("pwm")

# BME280 Sensor Examples
arduino_library(SparkFunBME280 "https://github.com/sparkfun/SparkFun_BME280_Arduino_Library" )
add_subdirectory("bme280-i2c")
add_subdirectory("bme280-spi")

if(USE_HTTPS)
    add_subdirectory("wifi-secure")
endif(USE_HTTPS)

