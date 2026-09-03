#include <Arduino.h>
#include <EthernetServer.h>
#include <SD.h>
#include <UDP.h>
#include <unity.h>

#include <array>
#include <cstring>
#include <filesystem>

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace arduino;

namespace {

class MockGPIO final : public HardwareGPIO {
 public:
  std::array<PinMode, 16> modes{};
  std::array<PinStatus, 16> digital{};
  std::array<int, 16> analog{};
  uint8_t reference = 0;
  int pwm_pin = -1;
  int pwm_value = -1;
  uint8_t resolution = 0;
  uint32_t frequency = 0;

  void pinMode(pin_size_t pin, PinMode mode) override { modes.at(pin) = mode; }
  void digitalWrite(pin_size_t pin, PinStatus status) override { digital.at(pin) = status; }
  PinStatus digitalRead(pin_size_t pin) override { return digital.at(pin); }
  int analogRead(pin_size_t pin) override { return analog.at(pin); }
  void analogReference(uint8_t mode) override { reference = mode; }
  void analogWrite(pin_size_t pin, int value) override {
    pwm_pin = pin;
    pwm_value = value;
  }
  void tone(uint8_t, unsigned int, unsigned long) override {}
  void noTone(uint8_t) override {}
  unsigned long pulseIn(uint8_t, uint8_t, unsigned long) override { return 123; }
  unsigned long pulseInLong(uint8_t, uint8_t, unsigned long) override { return 456; }
  void analogWriteFrequency(pin_size_t, uint32_t value) override { frequency = value; }
  void analogWriteResolution(uint8_t bits) override { resolution = bits; }
};

void test_timing() {
  const unsigned long before = millis();
  delay(15);
  TEST_ASSERT_TRUE(millis() - before >= 10);

  const unsigned long micros_before = micros();
  delayMicroseconds(1000);
  TEST_ASSERT_TRUE(micros() - micros_before >= 500);
}

void test_gpio_delegation() {
  MockGPIO mock;
  GPIO.setGPIO(&mock);

  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);
  analogWrite(4, 127);
  analogReference(2);
  analogWriteResolution(10);
  analogWriteFrequency(4, 1000);

  TEST_ASSERT_EQUAL(OUTPUT, mock.modes[3]);
  TEST_ASSERT_EQUAL(HIGH, digitalRead(3));
  TEST_ASSERT_EQUAL(4, mock.pwm_pin);
  TEST_ASSERT_EQUAL(127, mock.pwm_value);
  TEST_ASSERT_EQUAL_UINT8(2, mock.reference);
  TEST_ASSERT_EQUAL_UINT8(10, mock.resolution);
  TEST_ASSERT_EQUAL_UINT32(1000, mock.frequency);
  TEST_ASSERT_EQUAL_UINT32(123, pulseIn(3, HIGH, 100));
  TEST_ASSERT_EQUAL_UINT32(456, pulseInLong(3, HIGH, 100));

  GPIO.setGPIO(nullptr);
}

void test_arduino_types() {
  String value = String("Arduino") + " Emulator";
  TEST_ASSERT_EQUAL_UINT16(16, value.length());
  TEST_ASSERT_TRUE(value.startsWith("Arduino"));
  TEST_ASSERT_TRUE(value.endsWith("Emulator"));

  IPAddress address(192, 168, 1, 42);
  TEST_ASSERT_EQUAL_UINT8(192, address[0]);
  TEST_ASSERT_EQUAL_UINT8(42, address[3]);
  TEST_ASSERT_EQUAL_STRING("192.168.1.42", address.toString().c_str());
}

void test_ethernet_configuration() {
  uint8_t mac[] = {0, 1, 2, 3, 4, 5};
  const IPAddress local(10, 0, 0, 20);
  Ethernet.begin(mac, local, IPAddress(1, 1, 1, 1),
                 IPAddress(10, 0, 0, 1), IPAddress(255, 255, 255, 0));

  uint8_t returned_mac[6] = {};
  Ethernet.MACAddress(returned_mac);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(mac, returned_mac, 6);
  TEST_ASSERT_EQUAL(local, Ethernet.localIP());
  TEST_ASSERT_EQUAL(1, Ethernet.hardwareStatus());
  TEST_ASSERT_EQUAL(2, Ethernet.linkStatus());
}

void test_socket_error_mapping() {
#if defined(_WIN32)
  TEST_ASSERT_TRUE(socketWouldBlock(WSAEWOULDBLOCK));
  TEST_ASSERT_TRUE(socketConnectionReset(WSAECONNRESET));
#else
  TEST_ASSERT_TRUE(socketWouldBlock(EAGAIN));
  TEST_ASSERT_TRUE(socketConnectionReset(ECONNRESET));
#endif
}

void test_sd_filesystem() {
  const std::filesystem::path directory =
      std::filesystem::current_path() / "sd_unit_test_directory";
  const std::filesystem::path filename = directory / "record.txt";
  SD.remove(filename.string().c_str());
  SD.rmdir(directory.string().c_str());

  TEST_ASSERT_TRUE(SD.begin());
  TEST_ASSERT_TRUE(SD.mkdir(directory.string().c_str()));
  TEST_ASSERT_TRUE(SD.exists(directory.string().c_str()));

  File output = SD.open(filename.string().c_str(), O_WRITE);
  TEST_ASSERT_TRUE(output);
  const char content[] = "sd-emulator";
  TEST_ASSERT_EQUAL(sizeof(content) - 1,
                    output.write(reinterpret_cast<const uint8_t*>(content),
                                 sizeof(content) - 1));
  TEST_ASSERT_TRUE(output.close());

  File input = SD.open(filename.string().c_str(), O_READ);
  TEST_ASSERT_TRUE(input);
  char readback[sizeof(content)] = {};
  TEST_ASSERT_EQUAL(sizeof(content) - 1,
                    input.readBytes(readback, sizeof(readback) - 1));
  TEST_ASSERT_EQUAL_STRING(content, readback);
  TEST_ASSERT_TRUE(input.close());

  TEST_ASSERT_TRUE(SD.remove(filename.string().c_str()));
  TEST_ASSERT_TRUE(SD.rmdir(directory.string().c_str()));
  TEST_ASSERT_FALSE(SD.exists(filename.string().c_str()));
}

void test_udp_loopback() {
  constexpr uint16_t port = 45871;
  EthernetUDP receiver;
  EthernetUDP sender;
  TEST_ASSERT_EQUAL_UINT8(1, receiver.begin(port));
  TEST_ASSERT_EQUAL_UINT8(1, sender.begin(port + 1));

  const char payload[] = "udp-loopback";
  TEST_ASSERT_EQUAL(1, sender.beginPacket(IPAddress(127, 0, 0, 1), port));
  TEST_ASSERT_EQUAL(sizeof(payload) - 1, sender.write(reinterpret_cast<const uint8_t*>(payload), sizeof(payload) - 1));
  TEST_ASSERT_EQUAL(1, sender.endPacket());

  TEST_ASSERT_TRUE(receiver.parsePacket() > 0);
  char received[sizeof(payload)] = {};
  TEST_ASSERT_EQUAL(sizeof(payload) - 1, receiver.read(received, sizeof(received) - 1));
  TEST_ASSERT_EQUAL_STRING(payload, received);
  TEST_ASSERT_EQUAL_UINT16(port + 1, receiver.remotePort());
  receiver.stop();
  sender.stop();
}

void test_udp_send_then_receive_on_same_socket() {
  constexpr uint16_t simulator_port = 45873;
  constexpr uint16_t backend_port = 45874;
  EthernetUDP simulator;
  EthernetUDP backend;
  TEST_ASSERT_EQUAL_UINT8(1, simulator.begin(simulator_port));
  TEST_ASSERT_EQUAL_UINT8(1, backend.begin(backend_port));

  const char telemetry[] = "telemetry";
  TEST_ASSERT_EQUAL(1, simulator.beginPacket(IPAddress(127, 0, 0, 1), backend_port));
  TEST_ASSERT_EQUAL(sizeof(telemetry) - 1,
                    simulator.write(reinterpret_cast<const uint8_t*>(telemetry), sizeof(telemetry) - 1));
  TEST_ASSERT_EQUAL(1, simulator.endPacket());
  TEST_ASSERT_TRUE(backend.parsePacket() > 0);
  backend.flush();

  const uint8_t command[] = {'c', 0x00, 'm', 'm', 'a', 'n', 'd'};
  TEST_ASSERT_EQUAL(1, backend.beginPacket(IPAddress(127, 0, 0, 1), simulator_port));
  TEST_ASSERT_EQUAL(sizeof(command), backend.write(command, sizeof(command)));
  TEST_ASSERT_EQUAL(1, backend.endPacket());
  TEST_ASSERT_TRUE(simulator.parsePacket() > 0);
  char received[sizeof(command)] = {};
  TEST_ASSERT_EQUAL(sizeof(command), simulator.read(received, sizeof(received)));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(command, reinterpret_cast<const uint8_t*>(received), sizeof(command));
  TEST_ASSERT_EQUAL_UINT16(backend_port, simulator.remotePort());
  simulator.stop();
  backend.stop();
}

void test_tcp_loopback() {
  constexpr uint16_t port = 45872;
  EthernetServer server(port);
  server.begin();
  TEST_ASSERT_EQUAL(WL_CONNECTED, server.status());

  EthernetClient client;
  TEST_ASSERT_EQUAL(1, client.connect(IPAddress(127, 0, 0, 1), port));
  EthernetClient accepted = server.available();
  TEST_ASSERT_TRUE(accepted);

  const char payload[] = "tcp-loopback";
  TEST_ASSERT_EQUAL(sizeof(payload) - 1, client.write(reinterpret_cast<const uint8_t*>(payload), sizeof(payload) - 1));
  char received[sizeof(payload)] = {};
  TEST_ASSERT_EQUAL(sizeof(payload) - 1, accepted.readBytes(received, sizeof(received) - 1));
  TEST_ASSERT_EQUAL_STRING(payload, received);

  accepted.stop();
  client.stop();
  server.stop();
}

}  // namespace

void setUp() {}
void tearDown() {}

void setup_test() {
  UNITY_BEGIN();
  RUN_TEST(test_timing);
  RUN_TEST(test_gpio_delegation);
  RUN_TEST(test_arduino_types);
  RUN_TEST(test_ethernet_configuration);
  RUN_TEST(test_socket_error_mapping);
  RUN_TEST(test_sd_filesystem);
  RUN_TEST(test_udp_loopback);
  RUN_TEST(test_udp_send_then_receive_on_same_socket);
  RUN_TEST(test_tcp_loopback);
  UNITY_END();
}

static int run_tests() {
  setup_test();
  return 0;
}

#if defined(_WIN32)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return run_tests(); }
#else
int main() { return run_tests(); }
#endif
