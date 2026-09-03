#include <Arduino.h>

void setup() {
  Serial.println("Arduino Emulator native platform");
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, !digitalRead(13));
  delay(10);
}
