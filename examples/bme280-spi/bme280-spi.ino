// Example sketch for the BME280 combined humidity and pressure sensor
// Using SPI communication with the SparkFun BME280 Arduino Library

#include <Arduino.h>
#include <SPI.h>
#include "SparkFunBME280.h"

const int csPin = 13; // Chip select pin for SPI
BME280 mySensor;

void setup() {
	Serial.begin(9600);
	Serial.println("Reading basic values from BME280");
	// Begin communication over SPI. Use pin 13 as CS.
	if (!mySensor.beginSPI(csPin)) {
		Serial.println("The sensor did not respond. Please check wiring.");
		while (1);  // Freeze
	}
}

void loop() {
	Serial.print("Humidity: ");
	Serial.print(mySensor.readFloatHumidity(), 0);
	Serial.print(" Pressure: ");
	Serial.print(mySensor.readFloatPressure(), 0);
	Serial.print(" Alt: ");
	Serial.print(mySensor.readFloatAltitudeMeters(), 1);
	Serial.print(" Temp: ");
	Serial.println(mySensor.readTempC(), 2);

	delay(1000);
}
