int LED_BUILTIN = 13; // Define the built-in LED pin (usually pin 13 on Arduino boards)

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED pin as output
}

void loop() {
  Serial.println("Setting pin 13 HIGH");
  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED on
  delay(1000);                     // Wait for 1 second
  Serial.println("Setting pin 13 LOW");
  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off
  delay(1000);                     // Wait for 1 second
}