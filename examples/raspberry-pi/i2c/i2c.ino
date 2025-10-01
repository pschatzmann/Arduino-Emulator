/// Example for using I2C on Raspberry Pi

#include "Arduino.h"
#include "Wire.h"

const uint8_t i2cAddress = 0x3C; // Change to your I2C device address

void setup() {
  Serial.begin(115200);
  Serial.println("I2C Test Sketch Start");
  Wire.begin();
}

void loop() {
  // Write a byte (0xA5) to the I2C device
  Wire.beginTransmission(i2cAddress);
  Wire.write(0xA5);
  uint8_t result = Wire.endTransmission();
  Serial.print("Write result: ");
  Serial.println(result == 0 ? "Success" : "Error");

  delay(10);

  // Request 1 byte from the I2C device
  Wire.requestFrom(i2cAddress, (uint8_t)1);
  if (Wire.available()) {
    uint8_t data = Wire.read();
    Serial.print("Received: 0x");
    Serial.println(data, HEX);
  } else {
    Serial.println("No data received");
  }

  delay(1000);
}