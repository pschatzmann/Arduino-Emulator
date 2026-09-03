/*
  Network connect timing test.

  There are three test setups to exercise different situations:

  Instant connect failure:
    For this test, there should be nothing listening on localhost port 8123

    network-connect-timing should say "connect failed" with
    elapsed=0ms or nearly 0.

  Connect success:
    Start a plain TCP listener locally, for example:
      nc -lk 127.0.0.1 8123

    network-connect-timing should say "connect succeeded",
    display some send/response/connected/closed messages,
    and the elapsed time should be 1000 ms or so.

  Connect timeout:
    For this test, you need to configure the host kernel to drop packets.
    It does not matter if the listener is running or not.

    On MacOS you can drop packets as follows:
     Add this to /etc/pf.conf :
      anchor "connect-timeout-test"
      load anchor "connect-timeout-test" from "/etc/pf.anchors/connect-timeout-test"

     Create a file /etc/pf.anchors/connect-timeout-test containing:
      block drop quick on lo0 proto tcp from any to 127.0.0.1 port 8123
      block drop quick on lo0 proto tcp from 127.0.0.1 to any port 8123
    
     Then run this to enable the rules:
      sudo pfctl -f /etc/pf.conf              
      sudo pfctl -e

    For Linux you can drop packets by issuing:
      sudo nft add table inet connect_test
      sudo nft 'add chain inet connect_test output { type filter hook output priority 0; }'
      sudo nft 'add chain inet connect_test input { type filter hook input priority 0; }'
      sudo nft 'add rule inet connect_test output oifname "lo" tcp dport 8123 drop'
      sudo nft 'add rule inet connect_test input iifname "lo" tcp sport 8123 drop'

    With those packet-dropping rules in place,
     network-connect-test should say "connect failed"
     and the elapsed time should be a little more than 2000 ms
     for the first test and 3000 ms for the second.
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>

namespace {

constexpr const char* ssid = "localhost-test";
constexpr const char* password = "unused";
constexpr const char* host = "127.0.0.1";
constexpr uint16_t port = 8123;
constexpr int32_t explicit_connect_timeout_ms = 2000;
constexpr int32_t default_connect_timeout_ms = 3000;
constexpr unsigned long read_timeout_ms = 1000;

WiFiClient explicit_timeout_client;
WiFiClient default_timeout_client;

void print_test_elapsed(const char* label, unsigned long start_ms) {
  Serial.print(label);
  Serial.print(" elapsed=");
  Serial.print(millis() - start_ms);
  Serial.println("ms");
}

void wait_for_wifi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.println();
}

bool connect_with_explicit_timeout() {
  Serial.print("Connecting with explicit timeout to ");
  Serial.print(host);
  Serial.print(":");
  Serial.print(port);
  Serial.print(" timeout=");
  Serial.print(explicit_connect_timeout_ms);
  Serial.println("ms");

  if (!explicit_timeout_client.connect(host, port, explicit_connect_timeout_ms)) {
    Serial.println("Explicit-timeout connect failed");
    return false;
  }

  Serial.println("Explicit-timeout connect succeeded");
  return true;
}

bool connect_with_default_timeout() {
  default_timeout_client.setConnectionTimeout(default_connect_timeout_ms);

  Serial.print("Connecting with default connect(host, port) using timeout=");
  Serial.print(default_connect_timeout_ms);
  Serial.println("ms");

  if (!default_timeout_client.connect(host, port)) {
    Serial.println("Default-timeout connect failed");
    return false;
  }

  Serial.println("Default-timeout connect succeeded");
  return true;
}

void send_plain_text(WiFiClient& client, const char* label) {
  Serial.print(label);
  client.println("hello from WiFiClient");
}

void drain_input(WiFiClient& client, const char* label) {
  unsigned long deadline = millis() + read_timeout_ms;

  Serial.print(label);
  Serial.println(" waiting for response bytes");

  while (client.connected() && millis() < deadline) {
    while (client.available() > 0) {
      int ch = client.read();
      if (ch >= 0) {
        Serial.write(static_cast<char>(ch));
        deadline = millis() + read_timeout_ms;
      }
    }
    delay(10);
  }

  Serial.println();
  Serial.print(label);
  Serial.print(" connected() -> ");
  Serial.println(client.connected() ? "true" : "false");
}

void close_client(WiFiClient& client, const char* label) {
  client.stop();
  Serial.print(label);
  Serial.println(" closed");
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);

  wait_for_wifi();

  unsigned long explicit_test_start_ms = millis();
  if (connect_with_explicit_timeout()) {
    send_plain_text(explicit_timeout_client, "explicit_timeout_client");
    drain_input(explicit_timeout_client, "explicit_timeout_client");
    close_client(explicit_timeout_client, "explicit_timeout_client");
  }
  print_test_elapsed("explicit-timeout test", explicit_test_start_ms);
  Serial.println();

  unsigned long default_test_start_ms = millis();
  if (connect_with_default_timeout()) {
    send_plain_text(default_timeout_client, "default_timeout_client");
    drain_input(default_timeout_client, "default_timeout_client");
    close_client(default_timeout_client, "default_timeout_client");
  }
  print_test_elapsed("default-timeout test", default_test_start_ms);

  Serial.println();
  Serial.println("network-connect-timing test complete");
}

void loop() {
  exit(0);
  // delay(1000);
}
