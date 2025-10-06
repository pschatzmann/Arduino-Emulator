// pwm example
#include "Arduino.h"

// GPIO 12 should work on a PI and with FTDI FT2232HL for PWM
const int LED1_PIN = 12;   // GPIO 12

unsigned long lastStatsTime = 0;
const unsigned long STATS_INTERVAL = 5000;  // Print stats every 5 seconds

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  Serial.println("FTDI PWM Example Starting...");
  Serial.println("This example demonstrates software PWM on FTDI FT2232HL");
  Serial.println();
  
  // Configure PWM frequency (optional, default is 1000 Hz)
  analogWriteFrequency(LED1_PIN, 1000);
  
  // Configure pins as outputs
  pinMode(LED1_PIN, OUTPUT);
  
  Serial.println("PWM configured at 1000 Hz");
  Serial.println("Statistics will be displayed every 5 seconds");
  Serial.println();
}

void printPWMStatistics() {
  #ifdef USE_FTDI
  uint64_t cycles, max_jitter, avg_jitter;
  GPIO.getPWMStatistics(LED1_PIN, cycles, max_jitter, avg_jitter);
  
  if (cycles > 0) {
    Serial.println("\n--- PWM Statistics ---");
    Serial.print("Total cycles: ");
    Serial.println((unsigned long)cycles);
    Serial.print("Max jitter: ");
    Serial.print((unsigned long)max_jitter);
    Serial.println(" µs");
    Serial.print("Avg jitter: ");
    Serial.print((unsigned long)avg_jitter);
    Serial.println(" µs");
    
    // Warn if jitter is too high
    if (max_jitter > 100) {
      Serial.println("⚠️  WARNING: High jitter detected!");
      Serial.println("    Consider:");
      Serial.println("    - Running with elevated privileges for real-time priority");
      Serial.println("    - Reducing system load");
      Serial.println("    - Lowering PWM frequency");
    } else if (max_jitter < 50) {
      Serial.println("✓ Excellent PWM timing accuracy");
    }
    Serial.println("----------------------\n");
  }
  #endif
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
    
    // Check if it's time to print statistics
    unsigned long currentTime = millis();
    if (currentTime - lastStatsTime >= STATS_INTERVAL) {
      printPWMStatistics();
      lastStatsTime = currentTime;
    }
    
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
    
    // Check if it's time to print statistics
    unsigned long currentTime = millis();
    if (currentTime - lastStatsTime >= STATS_INTERVAL) {
      printPWMStatistics();
      lastStatsTime = currentTime;
    }
    
    delay(100);
  }
  
  delay(1000);  // Hold at minimum brightness 
}