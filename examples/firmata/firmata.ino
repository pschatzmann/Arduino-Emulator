// Firmata example: control GPIO pins on a device that speaks the Firmata
// protocol (e.g. a board running "StandardFirmata" from the Arduino IDE
// examples), reached through any arduino::Stream.
//
// HardwareGPIO_FIRMATA only needs a Stream - it does not care whether that
// stream is a real serial port, a TCP socket, or anything else. This example
// stands in a small in-process SimulatedFirmataDevice as the other end of
// the wire so the demo is fully self-contained and needs no extra hardware.
// To talk to a real board instead, replace `device` below with a Stream
// connected to its serial port and keep the rest of the sketch unchanged.
//
// Build with:
//   cmake -B build -DBUILD_EXAMPLES=ON -DUSE_FIRMATA=ON

#include "Arduino.h"
#include "HardwareGPIO_FIR.h"
#include <deque>

namespace {

// Firmata protocol command bytes (see HardwareGPIO_FIR.cpp for the same
// constants used on the host side).
constexpr uint8_t kDigitalMessage = 0x90;
constexpr uint8_t kAnalogMessage = 0xE0;
constexpr uint8_t kReportAnalog = 0xC0;
constexpr uint8_t kReportDigital = 0xD0;
constexpr uint8_t kSetPinMode = 0xF4;

/**
 * @brief Minimal stand-in for a remote Firmata peer, for demo purposes.
 *
 * Bytes written to this stream (host -> device) are interpreted the same
 * way a real Firmata sketch would. Once digital/analog reporting has been
 * requested, it periodically pushes simulated sensor readings back
 * (device -> host), exactly like a real board would after
 * REPORT_DIGITAL_PIN/REPORT_ANALOG_PIN is enabled.
 */
class SimulatedFirmataDevice : public Stream {
 public:
  size_t write(uint8_t b) override {
    switch (state_) {
      case State::kWaitCommand:
        if ((b & 0xF0) == kDigitalMessage) {
          pending_ = Pending::kDigitalWrite;
          channel_ = b & 0x0F;
          state_ = State::kWaitLsb;
        } else if (b == kSetPinMode) {
          pending_ = Pending::kSetPinMode;
          state_ = State::kWaitPin;
        } else if ((b & 0xF0) == kReportDigital) {
          digital_reporting_ = true;
        } else if ((b & 0xF0) == kReportAnalog) {
          analog_reporting_ = true;
          analog_channel_ = b & 0x0F;
        }
        break;
      case State::kWaitPin:
        pin_ = b;
        state_ = State::kWaitData;
        break;
      case State::kWaitLsb:
        lsb_ = b;
        state_ = State::kWaitData;
        break;
      case State::kWaitData:
        if (pending_ == Pending::kSetPinMode) {
          // b == Firmata pin mode; nothing to simulate, just acknowledged.
        } else {
          // Digital port write: report back what was just set so
          // digitalRead() on the host reflects the outgoing digitalWrite().
          const uint16_t portValue = lsb_ | (static_cast<uint16_t>(b) << 7);
          pushDigitalPort(channel_, portValue);
        }
        state_ = State::kWaitCommand;
        break;
    }
    return 1;
  }

  int available() override {
    return static_cast<int>(outbox_.size());
  }

  int read() override {
    if (outbox_.empty()) return -1;
    const int value = outbox_.front();
    outbox_.pop_front();
    return value;
  }

  int peek() override { return outbox_.empty() ? -1 : outbox_.front(); }

  /// Simulate the remote board pushing a fresh button + sensor reading.
  void simulateTick() {
    if (digital_reporting_) {
      button_state_ = !button_state_;
      pushDigitalPort(0, button_state_ ? 0x04 : 0x00);  // pin 2 on port 0
    }
    if (analog_reporting_) {
      analog_value_ = (analog_value_ + 37) % 1024;
      pushAnalog(analog_channel_, analog_value_);
    }
  }

 private:
  enum class State { kWaitCommand, kWaitPin, kWaitLsb, kWaitData };
  enum class Pending { kSetPinMode, kDigitalWrite };

  State state_ = State::kWaitCommand;
  Pending pending_ = Pending::kSetPinMode;
  uint8_t channel_ = 0;
  uint8_t pin_ = 0;
  uint8_t lsb_ = 0;

  bool digital_reporting_ = false;
  bool analog_reporting_ = false;
  uint8_t analog_channel_ = 0;
  bool button_state_ = false;
  int analog_value_ = 512;

  std::deque<uint8_t> outbox_;

  void pushDigitalPort(uint8_t port, uint16_t portValue) {
    outbox_.push_back(static_cast<uint8_t>(kDigitalMessage | port));
    outbox_.push_back(static_cast<uint8_t>(portValue & 0x7F));
    outbox_.push_back(static_cast<uint8_t>((portValue >> 7) & 0x7F));
  }

  void pushAnalog(uint8_t pin, int value) {
    outbox_.push_back(static_cast<uint8_t>(kAnalogMessage | pin));
    outbox_.push_back(static_cast<uint8_t>(value & 0x7F));
    outbox_.push_back(static_cast<uint8_t>((value >> 7) & 0x7F));
  }
};

const int LED_PIN = 13;
const int BUTTON_PIN = 2;
const int SENSOR_PIN = 0;

SimulatedFirmataDevice device;
HardwareGPIO_FIRMATA firmataGPIO;

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Firmata GPIO...");

  if (!firmataGPIO.begin(device)) {
    Serial.println("Failed to start Firmata GPIO");
    return;
  }

  // Route pinMode()/digitalWrite()/digitalRead()/analogRead() to the
  // Firmata backend instead of the default local GPIO implementation.
  GPIO.setGPIO(&firmataGPIO);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  Serial.println("Firmata GPIO ready");
}

void loop() {
  static bool led_state = false;
  led_state = !led_state;
  digitalWrite(LED_PIN, led_state ? HIGH : LOW);

  // Let the simulated remote board "report" its next button/sensor reading.
  device.simulateTick();
  delay(20);  // give the background reader thread time to parse it

  Serial.print("Button (pin ");
  Serial.print(BUTTON_PIN);
  Serial.print("): ");
  Serial.println(digitalRead(BUTTON_PIN) == HIGH ? "HIGH" : "LOW");

  Serial.print("Sensor (analog pin ");
  Serial.print(SENSOR_PIN);
  Serial.print("): ");
  Serial.println(analogRead(SENSOR_PIN));

  delay(500);
}
