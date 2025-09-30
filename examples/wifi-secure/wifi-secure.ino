/*
  Wifi secure connection example
*/

#include <Arduino.h>
#include <NetworkClientSecure.h>
#include <WiFi.h>
const char *ssid = "your-ssid";          // your network SSID (name of wifi network)
const char *password = "your-password";  // your network password
const char *server = "www.pschatzmann.ch";  // Server URL
NetworkClientSecure client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  delay(100);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);

  client.setInsecure(); // skip verification
  //client.setCACert(test_root_ca);
  //client.setCertificate(test_client_cert); // for client verification
  //client.setPrivateKey(test_client_key);	// for client verification

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443)) {
    Serial.println("Connection failed!");
  } else {
    Serial.println("Connected to server!");
    // Make a HTTP request:
    client.println("GET https://www.pschatzmann.ch/ HTTP/1.0");
    client.println("Host: www.howsmyssl.com");
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.print(line);
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }

    client.stop();
  }
  Serial.println("\nConnection closed.");
}

void loop() {
  // do nothing
}
