/// Example for using Serial2 on Raspberry Pi using GPIO 14 (TX) and GPIO 15 (RX)
 
#include "Arduino.h"

void setup() {
  Serial2.begin(115200); // Start serial communication at 115200 baud
}

void loop() {
  auto txt = "Hallo world from Serial2";
  Serial.println(txt);
  Serial2.println(txt);
  delay(1000); // Wait for 1 second
}