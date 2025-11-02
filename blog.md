# Arduino Emulator: Major Architecture Improvements and Raspberry Pi Support

*October 2025*

I'm excited to share some significant updates to the Arduino Emulator project that make it more robust, easier to use, and better aligned with official Arduino standards. These changes represent a major step forward in cross-platform Arduino development.

## 🔄 Arduino Core as Official Submodule

The most significant architectural change is **migrating to the official Arduino Core API as a git submodule**. Previously, we maintained our own fork of the Arduino Core, but now we point directly to the [official Arduino Core API repository](https://github.com/arduino/ArduinoCore-API) maintained by Arduino.

### Why This Matters:
- **Always up-to-date**: Automatic access to the latest Arduino API improvements and bug fixes
- **Perfect compatibility**: 100% alignment with official Arduino function signatures and behavior  
- **Reduced maintenance**: No need to manually sync API changes from upstream
- **Community trust**: Using the same core API that millions of Arduino developers rely on

This change ensures that your Arduino sketches will behave identically whether running on actual hardware or in the emulator.

## 🍓 Complete Raspberry Pi Hardware Support

The emulator now provides **full hardware abstraction for Raspberry Pi**, enabling real GPIO, SPI, I2C, and Serial communication through the actual hardware interfaces.

### What's New:
- **Real I2C communication** via `/dev/i2c-1` (GPIO 2/3)
- **Hardware SPI support** using `/dev/spidev0.0` and `/dev/spidev0.1`
- **Direct GPIO control** through the modern `gpiod` interface
- **Serial port access** to physical UART interfaces

### Build with Hardware Support:
```bash
# Enable Raspberry Pi hardware support
cmake -DUSE_RPI=ON ..
make -j4

# Run with hardware access
sudo ./examples/i2c/i2c
```

### Before vs. After:
```cpp
// This I2C code now generates REAL signals on GPIO 2/3
Wire.begin();
Wire.beginTransmission(0x3C);
Wire.write(0xA5);
Wire.endTransmission();  // Actually sends data to hardware!
```

## 🎯 Redesigned Hardware Abstraction Layer

The GPIO, SPI, and I2C implementations have been **completely redesigned** for consistency and extensibility:

### New Architecture Benefits:
- **Unified interface**: All hardware abstractions follow the same design patterns
- **Multiple backends**: Easy switching between Mock, Raspberry Pi, FTDI, and Remote implementations  
- **Clean separation**: Hardware-specific code is isolated and modular
- **Future-proof**: Easy to add support for new platforms

### Implementation Options:
- **Mock**: For testing and development without hardware
- **Raspberry Pi**: Direct hardware access via Linux kernel interfaces
- **FTDI**: USB-to-GPIO/SPI/I2C bridge support
- **Remote**: Communicate with real Arduino via UDP/Serial

## 🛠️ Simplified CMake Integration

Building Arduino sketches and integrating libraries is now **incredibly simple** with new CMake functions:

### arduino_sketch() Function
```cmake
# Before: Complex manual configuration
add_executable(blink blink.ino)
set_source_files_properties(blink.ino PROPERTIES LANGUAGE CXX)
target_compile_options(blink PRIVATE -x c++)
target_link_libraries(blink arduino_emulator)
# ... many more lines ...

# After: One simple line!
arduino_sketch(blink blink.ino)
```

### arduino_library() Function
```cmake
# Automatically download and integrate Arduino libraries
arduino_library(sam "https://github.com/pschatzmann/arduino-SAM")
arduino_sketch(my_project main.ino LIBRARIES sam)

# Or use local libraries
arduino_library(my_sensors ./local/sensors)
arduino_sketch(sensor_demo demo.ino LIBRARIES my_sensors Wire)
```

## 📚 Comprehensive Documentation

All these improvements are thoroughly documented in our **[Wiki](https://github.com/pschatzmann/Arduino-Emulator/wiki)**, including:

- **Getting Started Guide**: From zero to running sketches in minutes
- **Hardware Setup**: Detailed Raspberry Pi configuration instructions  
- **CMake Reference**: Complete function documentation with examples
- **Architecture Overview**: Understanding the emulator's design
- **Platform Support**: Comparison of different backend implementations
- **Troubleshooting**: Common issues and solutions

## 🚀 What This Means for You

These changes make the Arduino Emulator more powerful and easier to use than ever:

### For Developers:
- **Faster iteration**: Test Arduino code without uploading to hardware
- **Better debugging**: Use full IDE debugging capabilities  
- **Cross-platform**: Develop on Linux, macOS, or Windows
- **CI/CD integration**: Automated testing of Arduino projects

### For Raspberry Pi Users:
- **Real hardware testing**: Verify I2C/SPI communication with actual devices
- **Prototype faster**: Test sensor integration before building circuits
- **Educational tool**: Learn Arduino programming with immediate hardware feedback

### For Project Maintainers:
- **Simplified builds**: CMake integration reduces configuration complexity
- **Library ecosystem**: Easy integration with existing Arduino libraries
- **Future-proof**: Automatic compatibility with Arduino API evolution

## � FTDI USB-to-GPIO Bridge Support

One of the exciting next planned additions is **comprehensive FTDI FT2232HL support**, bringing real Arduino hardware capabilities to any desktop computer via USB.


## Try It Now!

Ready to experience the improved Arduino Emulator? Check out the complete documentation in our [Wiki](https://github.com/pschatzmann/Arduino-Emulator/wiki) and start building cross-platform Arduino applications today!

```bash
git clone --recursive https://github.com/pschatzmann/Arduino-Emulator.git
cd Arduino-Emulator
mkdir build && cd build
cmake -DUSE_RPI=ON ..  # Enable Raspberry Pi support
make -j4
./examples/blink/blink  # Your first emulated Arduino sketch!
```

---

*The Arduino Emulator continues to bridge the gap between embedded development and modern software practices. These improvements make it an even more powerful tool for Arduino developers working on complex, cross-platform projects.*