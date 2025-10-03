/// Example for using SPI over UDP
#include "Arduino.h"
#include "SPI.h"

SPISettings settings(1000000, MSBFIRST, SPI_MODE0); // 1 MHz, MSB first, Mode 0
const int csPin = 8;

void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud

  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);
}

void loop() {
  uint8_t txBuf[4] = {0x01, 0x02, 0x03, 0x04};
  uint8_t rxBuf[4] = {0};

  digitalWrite(csPin, LOW);
  SPI.beginTransaction(settings);
  SPI.transfer(txBuf, 4); // This will overwrite txBuf with received data
  SPI.endTransaction();
  digitalWrite(csPin, HIGH);

  Serial.print("Received: ");
  for (int i = 0; i < 4; ++i) {
    Serial.print("0x");
    Serial.print(txBuf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  delay(1000);
}