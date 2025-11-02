

#include "HardwareGPIO_FIR.h"
#include "Stream.h" // or <Stream.h> if needed
#include <map>
#include <vector>
#include <cstdint>

namespace arduino {


Stream* firmataStream = nullptr;
std::map<pin_size_t, PinMode> pinModes;
std::map<pin_size_t, PinStatus> pinStates;
std::map<pin_size_t, int> analogValues;
std::vector<uint8_t> rxBuffer;

// Firmata protocol constants
constexpr uint8_t DIGITAL_MESSAGE = 0x90;
constexpr uint8_t ANALOG_MESSAGE = 0xE0;
constexpr uint8_t SET_PIN_MODE = 0xF4;
constexpr uint8_t REPORT_DIGITAL = 0xD0;
constexpr uint8_t REPORT_ANALOG = 0xC0;

HardwareGPIO_FIRMATA::~HardwareGPIO_FIRMATA() {
    end();
}

bool HardwareGPIO_FIRMATA::begin(Stream &stream) {
    firmataStream = &stream;
    is_open = true;
    rxBuffer.clear();
    return true;
}

void HardwareGPIO_FIRMATA::end() {
    is_open = false;
    firmataStream = nullptr;
    pinModes.clear();
    pinStates.clear();
    analogValues.clear();
    rxBuffer.clear();
}

void HardwareGPIO_FIRMATA::pinMode(pin_size_t pinNumber, PinMode pinMode) {
    pinModes[pinNumber] = pinMode;
    if (firmataStream) {
        firmataStream->write(SET_PIN_MODE);
        firmataStream->write(pinNumber);
        firmataStream->write(pinMode);
    }
}

void HardwareGPIO_FIRMATA::digitalWrite(pin_size_t pinNumber, PinStatus status) {
    pinStates[pinNumber] = status;
    if (firmataStream) {
        uint8_t port = pinNumber / 8;
        uint8_t portValue = 0;
        for (int i = 0; i < 8; ++i) {
            pin_size_t p = port * 8 + i;
            if (pinStates[p] == HIGH) {
                portValue |= (1 << i);
            }
        }
        firmataStream->write(DIGITAL_MESSAGE | port);
        firmataStream->write(portValue & 0x7F);
        firmataStream->write((portValue >> 7) & 0x7F);
    }
}

PinStatus HardwareGPIO_FIRMATA::digitalRead(pin_size_t pinNumber) {
    // Request digital report for the port
    if (firmataStream) {
        uint8_t port = pinNumber / 8;
        firmataStream->write(REPORT_DIGITAL | port);
        firmataStream->write(1); // enable reporting
    }
    // Try to parse digital message from rxBuffer
    for (size_t i = 0; i + 2 < rxBuffer.size(); ++i) {
        uint8_t msg = rxBuffer[i];
        if ((msg & 0xF0) == DIGITAL_MESSAGE) {
            uint8_t port = msg & 0x0F;
            uint8_t lsb = rxBuffer[i+1];
            uint8_t msb = rxBuffer[i+2];
            uint16_t portValue = lsb | (msb << 7);
            pin_size_t p = port * 8;
            for (int j = 0; j < 8; ++j) {
                pinStates[p + j] = (portValue & (1 << j)) ? HIGH : LOW;
            }
            // Remove processed message
            rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + i + 3);
            break;
        }
    }
    auto it = pinStates.find(pinNumber);
    if (it != pinStates.end()) {
        return it->second;
    }
    return LOW;
}

int HardwareGPIO_FIRMATA::analogRead(pin_size_t pinNumber) {
    // Request analog report for the pin
    if (firmataStream) {
        firmataStream->write(REPORT_ANALOG | (pinNumber & 0x0F));
        firmataStream->write(1); // enable reporting
    }
    // Try to parse analog message from rxBuffer
    for (size_t i = 0; i + 2 < rxBuffer.size(); ++i) {
        uint8_t msg = rxBuffer[i];
        if ((msg & 0xF0) == ANALOG_MESSAGE) {
            uint8_t pin = msg & 0x0F;
            uint8_t lsb = rxBuffer[i+1];
            uint8_t msb = rxBuffer[i+2];
            int value = lsb | (msb << 7);
            analogValues[pin] = value;
            // Remove processed message
            rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + i + 3);
            break;
        }
    }
    auto it = analogValues.find(pinNumber);
    if (it != analogValues.end()) {
        return it->second;
    }
    return 0;
}

void HardwareGPIO_FIRMATA::analogReference(uint8_t mode) {
    // Not supported by Firmata, do nothing
}

void HardwareGPIO_FIRMATA::analogWrite(pin_size_t pinNumber, int value) {
    if (firmataStream) {
        firmataStream->write(ANALOG_MESSAGE | (pinNumber & 0x0F));
        firmataStream->write(value & 0x7F);
        firmataStream->write((value >> 7) & 0x7F);
    }
}

void HardwareGPIO_FIRMATA::analogWriteFrequency(pin_size_t pinNumber, uint32_t frequency) {
    // Not supported by Firmata, do nothing
}

void HardwareGPIO_FIRMATA::tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
    // Not supported by Firmata, do nothing
}

void HardwareGPIO_FIRMATA::noTone(uint8_t _pin) {
    // Not supported by Firmata, do nothing
}

unsigned long HardwareGPIO_FIRMATA::pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
    // Not supported by Firmata, return 0
    return 0;
}

unsigned long HardwareGPIO_FIRMATA::pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout) {
    // Not supported by Firmata, return 0
    return 0;
}

void HardwareGPIO_FIRMATA::analogWriteResolution(uint8_t bits) {
    // Firmata supports 8-bit resolution, do nothing
}

} // namespace arduino
