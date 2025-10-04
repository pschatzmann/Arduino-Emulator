// pwm example
#include "Arduino.h"

// Pin definitions for FTDI FT2232HL
const int LED1_PIN = 0;   // Channel A, pin 0 (ADBUS0)

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  Serial.println("FTDI PWM Example Starting...");
  Serial.println("This example demonstrates software PWM on FTDI FT2232HL");
  Serial.println();
  
  // Configure pins as outputs
  pinMode(LED1_PIN, OUTPUT);
}

void loop() {
  Serial.println("=== PWM Fade Cycle ===");
  
  // Fade all LEDs from 0 to 255 (0% to 100% duty cycle)
  Serial.println("Fading UP (0% -> 100%)");
  for (int brightness = 0; brightness <= 255; brightness += 5) {
    analogWrite(LED1_PIN, brightness);
    
    Serial.print("Brightness: ");
    Serial.print(brightness);
    Serial.print("/255 (");
    Serial.print((brightness * 100.0f) / 255.0f);
    Serial.println("%)");
    
    delay(100);  // Small delay to see the fade effect
  }
  
  delay(1000);  // Hold at maximum brightness
  
  // Fade all LEDs from 255 to 0 (100% to 0% duty cycle)
  Serial.println("Fading DOWN (100% -> 0%)");
  for (int brightness = 255; brightness >= 0; brightness -= 5) {
    analogWrite(LED1_PIN, brightness);
    
    Serial.print("Brightness: ");
    Serial.print(brightness);
    Serial.print("/255 (");
    Serial.print((brightness * 100.0f) / 255.0f);
    Serial.println("%)");
    
    delay(100);
  }
  
  delay(1000);  // Hold at minimum brightness 
}