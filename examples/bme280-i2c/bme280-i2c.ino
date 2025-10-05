// Example sketch for the BME280 combined humidity and pressure sensor
// Using I2C communication with the SparkFun BME280 Arduino Library

#include <Arduino.h>
#include <Wire.h>
#include "SparkFunBME280.h"

BME280 mySensor;

void setup() {
	Serial.begin(115200);
	Serial.println("Reading basic values from BME280");
	// Begin communication over I2C
	Wire.begin();
	if (mySensor.beginI2C() == false) {
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
