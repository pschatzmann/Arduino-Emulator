/*
  HardwareGPIO_FTDI.cpp 
  Copyright (c) 2025 Phil Schatzmann. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/
#ifdef USE_FTDI
#include "HardwareGPIO_FTDI.h"
#include "Arduino.h"

namespace arduino {

HardwareGPIO_FTDI::~HardwareGPIO_FTDI() {
  end();
}

bool HardwareGPIO_FTDI::begin(int vendor_id, int product_id, 
                              const char* description, const char* serial) {
  Logger.info("Initializing FTDI GPIO interface");
  
  // Initialize FTDI context
  ftdi_context = ftdi_new();
  if (!ftdi_context) {
    Logger.error("Failed to create FTDI context");
    return false;
  }

  // Open device
  int ret;
  if (serial) {
    ret = ftdi_usb_open_desc(ftdi_context, vendor_id, product_id, description, serial);
  } else if (description) {
    ret = ftdi_usb_open_desc(ftdi_context, vendor_id, product_id, description, nullptr);
  } else {
    ret = ftdi_usb_open(ftdi_context, vendor_id, product_id);
  }

  if (ret < 0) {
    Logger.error("Failed to open FTDI device: %s", ftdi_get_error_string(ftdi_context));
    ftdi_free(ftdi_context);
    ftdi_context = nullptr;
    return false;
  }

  // Reset device
  ret = ftdi_usb_reset(ftdi_context);
  if (ret < 0) {
    Logger.error("Failed to reset FTDI device: %s", ftdi_get_error_string(ftdi_context));
    end();
    return false;
  }

  // Set bitbang mode for GPIO
  // Enable all pins as outputs initially (will be changed by pinMode)
  ret = ftdi_set_bitmode(ftdi_context, 0xFF, BITMODE_BITBANG);
  if (ret < 0) {
    Logger.error("Failed to set bitbang mode: %s", ftdi_get_error_string(ftdi_context));
    end();
    return false;
  }

  // Set reasonable baud rate for GPIO operations
  ret = ftdi_set_baudrate(ftdi_context, 9600);
  if (ret < 0) {
    Logger.error("Failed to set baud rate: %s", ftdi_get_error_string(ftdi_context));
    end();
    return false;
  }

  // Set low latency timer for better PWM performance (default is 16ms, set to 1ms)
  ret = ftdi_set_latency_timer(ftdi_context, 1);
  if (ret < 0) {
    Logger.warning("Failed to set latency timer: %s", ftdi_get_error_string(ftdi_context));
  }
  
  // Enable USB transfer chunking for better performance
  ftdi_write_data_set_chunksize(ftdi_context, 256);
  ftdi_read_data_set_chunksize(ftdi_context, 256);

  is_open = true;
  Logger.info("FTDI GPIO interface initialized successfully");
  return true;
}

void HardwareGPIO_FTDI::end() {
  // Stop PWM thread first
  stopPWMThread();
  
  if (ftdi_context) {
    ftdi_usb_close(ftdi_context);
    ftdi_free(ftdi_context);
    ftdi_context = nullptr;
  }
  is_open = false;
  pin_modes.clear();
  pwm_pins.clear();
}

void HardwareGPIO_FTDI::pinMode(pin_size_t pinNumber, PinMode pinMode) {
  if (!is_open || pinNumber > 15) {
    Logger.error("Invalid pin number or FTDI not initialized");
    return;
  }

  pin_modes[pinNumber] = pinMode;
  int channel = getChannel(pinNumber);
  int bit_pos = getBitPosition(pinNumber);
  
  if (channel == 0) {
    // Channel A (pins 0-7)
    if (pinMode == OUTPUT) {
      pin_directions_a |= (1 << bit_pos);   // Set as output
    } else {
      pin_directions_a &= ~(1 << bit_pos);  // Set as input
    }
    updateGPIOState(0);
  } else {
    // Channel B (pins 8-15)
    if (pinMode == OUTPUT) {
      pin_directions_b |= (1 << bit_pos);   // Set as output
    } else {
      pin_directions_b &= ~(1 << bit_pos);  // Set as input
    }
    updateGPIOState(1);
  }
}

void HardwareGPIO_FTDI::digitalWrite(pin_size_t pinNumber, PinStatus status) {
  if (!is_open || pinNumber > 15) {
    Logger.error("Invalid pin number or FTDI not initialized");
    return;
  }

  // Check if pin is configured as output
  auto it = pin_modes.find(pinNumber);
  if (it == pin_modes.end() || it->second != OUTPUT) {
    Logger.warning("Pin not configured as output");
    return;
  }

  int channel = getChannel(pinNumber);
  int bit_pos = getBitPosition(pinNumber);
  
  if (channel == 0) {
    // Channel A (pins 0-7)
    if (status == HIGH) {
      pin_values_a |= (1 << bit_pos);
    } else {
      pin_values_a &= ~(1 << bit_pos);
    }
    updateGPIOState(0);
  } else {
    // Channel B (pins 8-15)  
    if (status == HIGH) {
      pin_values_b |= (1 << bit_pos);
    } else {
      pin_values_b &= ~(1 << bit_pos);
    }
    updateGPIOState(1);
  }
}

PinStatus HardwareGPIO_FTDI::digitalRead(pin_size_t pinNumber) {
  if (!is_open || pinNumber > 15) {
    Logger.error("Invalid pin number or FTDI not initialized");
    return LOW;
  }

  int channel = getChannel(pinNumber);
  int bit_pos = getBitPosition(pinNumber);
  uint8_t value;
  
  if (!readGPIOState(channel, value)) {
    return LOW;
  }
  
  return (value & (1 << bit_pos)) ? HIGH : LOW;
}

int HardwareGPIO_FTDI::analogRead(pin_size_t pinNumber) {
  Logger.warning("analogRead not supported by FTDI FT2232HL");
  return 0;
}

void HardwareGPIO_FTDI::analogReference(uint8_t mode) {
  Logger.warning("analogReference not supported by FTDI FT2232HL");
}

void HardwareGPIO_FTDI::analogWrite(pin_size_t pinNumber, int value) {
  if (!is_open || pinNumber > 15) {
    Logger.error("Invalid pin number or FTDI not initialized");
    return;
  }

  // Clamp value to valid range
  if (value < 0) value = 0;
  if (value > 255) value = 255;
  
  // Set pin as output if not already
  if (pin_modes[pinNumber] != OUTPUT) {
    pinMode(pinNumber, OUTPUT);
  }

  if (value == 0) {
    // 0% duty cycle - turn off PWM and set pin LOW
    {
      std::lock_guard<std::mutex> lock(pwm_mutex);
      pwm_pins[pinNumber].enabled = false;
    }
    digitalWrite(pinNumber, LOW);
  } else if (value == 255) {
    // 100% duty cycle - turn off PWM and set pin HIGH  
    {
      std::lock_guard<std::mutex> lock(pwm_mutex);
      pwm_pins[pinNumber].enabled = false;
    }
    digitalWrite(pinNumber, HIGH);
  } else {
    // Enable PWM for this pin
    uint32_t frequency = 1000;  // Default 1 kHz PWM frequency
    
    // Check if pin already has a custom frequency set
    {
      std::lock_guard<std::mutex> lock(pwm_mutex);
      auto it = pwm_pins.find(pinNumber);
      if (it != pwm_pins.end() && it->second.frequency > 0) {
        frequency = it->second.frequency;  // Use existing frequency
      }
    }
    
    updatePWMPin(pinNumber, value, frequency);
    
    // Start PWM thread if not already running
    if (!pwm_thread_running) {
      startPWMThread();
    }
  }
}

void HardwareGPIO_FTDI::analogWriteFrequency(pin_size_t pinNumber, uint32_t frequency) {
  String pin{pinNumber};
  String freq{frequency};
  Logger.debug("analogWriteFrequency:", pin.c_str(), freq.c_str());
  
  if (!is_open || pinNumber > 15) {
    Logger.error("Invalid pin number or FTDI not initialized");
    return;
  }
  
  // Limit to reasonable range - higher frequencies will have poor accuracy due to USB latency
  if (frequency == 0 || frequency > 10000) {
    Logger.error("Invalid PWM frequency (valid range: 1-10000 Hz for reliable operation)");
    return;
  }
  
  // Update frequency for existing PWM pin or set it for future use
  {
    std::lock_guard<std::mutex> lock(pwm_mutex);
    PWMPin& pwm = pwm_pins[pinNumber];
    uint8_t current_duty = pwm.duty_cycle;
    bool was_enabled = pwm.enabled;
    
    // Update the frequency
    pwm.frequency = frequency;
    pwm.period_us = 1000000 / frequency;  // Convert Hz to microseconds
    
    // If PWM was active, recalculate timing
    if (was_enabled) {
      pwm.on_time_us = (pwm.period_us * current_duty) / 255;
      pwm.period_start = std::chrono::high_resolution_clock::now();
      Logger.debug("Updated PWM frequency for active pin");
    }
  }
}

void HardwareGPIO_FTDI::tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
  Logger.warning("tone generation not supported by FTDI FT2232HL");
}

void HardwareGPIO_FTDI::noTone(uint8_t _pin) {
  Logger.warning("noTone not supported by FTDI FT2232HL");
}

unsigned long HardwareGPIO_FTDI::pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
  Logger.warning("pulseIn not implemented for FTDI FT2232HL");
  return 0;
}

unsigned long HardwareGPIO_FTDI::pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout) {
  Logger.warning("pulseInLong not implemented for FTDI FT2232HL");
  return 0;
}

bool HardwareGPIO_FTDI::updateGPIOState(int channel) {
  if (!is_open) {
    return false;
  }

  int ret;
  if (channel == 0) {
    // Update Channel A
    ret = ftdi_set_bitmode(ftdi_context, pin_directions_a, BITMODE_BITBANG);
    if (ret < 0) {
      Logger.error("Failed to set Channel A direction: %s", ftdi_get_error_string(ftdi_context));
      return false;
    }
    
    // Write the output values
    ret = ftdi_write_data(ftdi_context, &pin_values_a, 1);
    if (ret < 0) {
      Logger.error("Failed to write Channel A values: %s", ftdi_get_error_string(ftdi_context));
      return false;
    }
  } else {
    // For Channel B, we need to select it first
    ret = ftdi_set_interface(ftdi_context, INTERFACE_B);
    if (ret < 0) {
      Logger.error("Failed to select Channel B: %s", ftdi_get_error_string(ftdi_context));
      return false;
    }
    
    ret = ftdi_set_bitmode(ftdi_context, pin_directions_b, BITMODE_BITBANG);
    if (ret < 0) {
      Logger.error("Failed to set Channel B direction: %s", ftdi_get_error_string(ftdi_context));
      return false;
    }
    
    ret = ftdi_write_data(ftdi_context, &pin_values_b, 1);
    if (ret < 0) {
      Logger.error("Failed to write Channel B values: %s", ftdi_get_error_string(ftdi_context));
      return false;
    }
    
    // Switch back to Channel A
    ftdi_set_interface(ftdi_context, INTERFACE_A);
  }
  
  return true;
}

bool HardwareGPIO_FTDI::readGPIOState(int channel, uint8_t& value) {
  if (!is_open) {
    return false;
  }

  int ret;
  if (channel == 1) {
    // Select Channel B
    ret = ftdi_set_interface(ftdi_context, INTERFACE_B);
    if (ret < 0) {
      Logger.error("Failed to select Channel B: %s", ftdi_get_error_string(ftdi_context));
      return false;
    }
  }
  
  // Read the pin states
  ret = ftdi_read_pins(ftdi_context, &value);
  if (ret < 0) {
    Logger.error("Failed to read pin states: %s", ftdi_get_error_string(ftdi_context));
    if (channel == 1) {
      ftdi_set_interface(ftdi_context, INTERFACE_A); // Switch back
    }
    return false;
  }
  
  if (channel == 1) {
    // Switch back to Channel A
    ftdi_set_interface(ftdi_context, INTERFACE_A);
  }
  
  return true;
}

void HardwareGPIO_FTDI::pwmThreadFunction() {
  Logger.info("PWM thread started");
  
  while (pwm_thread_running) {
    auto current_time = std::chrono::high_resolution_clock::now();
    bool channel_a_changed = false;
    bool channel_b_changed = false;
    
    {
      std::lock_guard<std::mutex> lock(pwm_mutex);
      
      // Process each PWM pin
      for (auto& pair : pwm_pins) {
        pin_size_t pin = pair.first;
        PWMPin& pwm = pair.second;
        
        if (!pwm.enabled) continue;
        
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
          current_time - pwm.period_start).count();
        
        bool new_state = false;
        bool state_change = false;
        
        if (elapsed >= pwm.period_us) {
          // New period starts
          // Calculate jitter for statistics
          uint64_t expected_period_time = pwm.cycle_count * pwm.period_us;
          auto total_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            current_time - pwm.period_start).count();
          uint64_t jitter = (total_elapsed > expected_period_time) ? 
                           (total_elapsed - expected_period_time) : 
                           (expected_period_time - total_elapsed);
          
          pwm.total_jitter_us += jitter;
          if (jitter > pwm.max_jitter_us) {
            pwm.max_jitter_us = jitter;
          }
          
          pwm.period_start = current_time;
          pwm.cycle_count++;
          new_state = (pwm.on_time_us > 0);
          state_change = (new_state != pwm.current_state);
        } else if (pwm.current_state && elapsed >= pwm.on_time_us) {
          // Transition HIGH to LOW
          new_state = false;
          state_change = true;
        } else {
          new_state = pwm.current_state;
        }
        
        if (state_change) {
          pwm.current_state = new_state;
          int channel = getChannel(pin);
          int bit_pos = getBitPosition(pin);
          
          if (channel == 0) {
            if (new_state) {
              pin_values_a |= (1 << bit_pos);
            } else {
              pin_values_a &= ~(1 << bit_pos);
            }
            channel_a_changed = true;
          } else {
            if (new_state) {
              pin_values_b |= (1 << bit_pos);
            } else {
              pin_values_b &= ~(1 << bit_pos);
            }
            channel_b_changed = true;
          }
        }
      }
    }
    
    // Update only changed channels to reduce USB overhead
    if (channel_a_changed) {
      updateGPIOState(0);
    }
    if (channel_b_changed) {
      updateGPIOState(1);
    }
    
    // Dynamic sleep time based on active PWM frequencies
    // Sleep for a fraction of the minimum period to ensure responsive timing
    auto min_period = std::chrono::microseconds::max();
    {
      std::lock_guard<std::mutex> lock(pwm_mutex);
      for (const auto& pair : pwm_pins) {
        if (pair.second.enabled) {
          auto period = std::chrono::microseconds(pair.second.period_us);
          min_period = std::min(min_period, period);
        }
      }
    }
    
    if (min_period != std::chrono::microseconds::max()) {
      // Sleep for 1% of minimum period, but at least 1µs and at most 100µs
      auto sleep_time = std::max(
        std::chrono::microseconds(1),
        std::min(
          std::chrono::microseconds(100),
          min_period / 100
        )
      );
      std::this_thread::sleep_for(sleep_time);
    } else {
      // No active PWM pins, sleep longer
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
  }
  
  Logger.info("PWM thread stopped");
}

void HardwareGPIO_FTDI::startPWMThread() {
  if (pwm_thread_running) return;
  
  pwm_thread_running = true;
  pwm_thread = std::thread(&HardwareGPIO_FTDI::pwmThreadFunction, this);
  
  // Set real-time priority for better timing accuracy (Linux only)
  #ifdef __linux__
  struct sched_param param;
  param.sched_priority = sched_get_priority_max(SCHED_FIFO) - 1; // High but not max priority
  if (pthread_setschedparam(pwm_thread.native_handle(), SCHED_FIFO, &param) != 0) {
    Logger.warning("Failed to set real-time priority for PWM thread (requires CAP_SYS_NICE capability or root)");
    Logger.warning("PWM timing may be less accurate. Consider running with elevated privileges for production use.");
  } else {
    Logger.info("PWM thread running with real-time priority");
  }
  #endif
  
  Logger.info("PWM thread started");
}

void HardwareGPIO_FTDI::stopPWMThread() {
  if (!pwm_thread_running) return;
  
  pwm_thread_running = false;
  if (pwm_thread.joinable()) {
    pwm_thread.join();
  }
  Logger.info("PWM thread stopped");
}

void HardwareGPIO_FTDI::updatePWMPin(pin_size_t pin, uint8_t duty_cycle, uint32_t frequency) {
  std::lock_guard<std::mutex> lock(pwm_mutex);
  
  PWMPin& pwm = pwm_pins[pin];
  pwm.enabled = true;
  pwm.duty_cycle = duty_cycle;
  pwm.frequency = frequency;
  pwm.period_us = 1000000 / frequency;  // Convert Hz to microseconds
  pwm.on_time_us = (pwm.period_us * duty_cycle) / 255;  // Calculate on-time based on duty cycle
  pwm.current_state = false;
  pwm.period_start = std::chrono::high_resolution_clock::now();
  pwm.cycle_count = 0;
  pwm.max_jitter_us = 0;
  pwm.total_jitter_us = 0;
  
  Logger.debug("PWM pin configured");
}

void HardwareGPIO_FTDI::getPWMStatistics(pin_size_t pin, uint64_t& cycles, 
                                         uint64_t& max_jitter_us, uint64_t& avg_jitter_us) {
  std::lock_guard<std::mutex> lock(pwm_mutex);
  auto it = pwm_pins.find(pin);
  if (it != pwm_pins.end() && it->second.enabled) {
    cycles = it->second.cycle_count;
    max_jitter_us = it->second.max_jitter_us;
    avg_jitter_us = (cycles > 0) ? it->second.total_jitter_us / cycles : 0;
  } else {
    cycles = 0;
    max_jitter_us = 0;
    avg_jitter_us = 0;
  }
}

void HardwareGPIO_FTDI::analogWriteResolution(uint8_t bits) {
  // FTDI FT2232HL supports 8-bit PWM resolution (0-255)
  // Log a warning if user tries to set different resolution
  if (bits != 8) {
    Logger.warning("FTDI FT2232HL only supports 8-bit PWM resolution (0-255)");
  }
  // For FTDI implementation, we always use 8-bit resolution internally
}

}  // namespace arduino

#endif  // USE_FTDI