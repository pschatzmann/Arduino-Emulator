#include "HardwareSPI.h"
#include "HardwareService.h"
#include "Hardware.h"
/**
 * We suppport different implementations for the SPI
 *
 **/

namespace arduino {

//HardwareSPI::HardwareSPI() {}

//HardwareSPI::~HardwareSPI() {}

uint8_t HardwareSPI::transfer(uint8_t data) {
    if (Hardware.spi != nullptr) {
        return Hardware.spi->transfer(data);
    } else {
        return 0;
    }
}

uint16_t HardwareSPI::transfer16(uint16_t data) {
    if (Hardware.spi != nullptr) {
        return Hardware.spi->transfer16(data);
    } else {
        return 0;
    }
}

void HardwareSPI::transfer(void *data, size_t count) {
    if (Hardware.spi != nullptr) {
        Hardware.spi->transfer(data, count);
    }
}

void HardwareSPI::usingInterrupt(int interruptNumber) {
    if (Hardware.spi != nullptr) {
        Hardware.spi->usingInterrupt(interruptNumber);
    }
}

void HardwareSPI::notUsingInterrupt(int interruptNumber){
    if (Hardware.spi != nullptr) {
        Hardware.spi->notUsingInterrupt(interruptNumber);
    }
}

void HardwareSPI::beginTransaction(SPISettings settings){
    if (Hardware.spi != nullptr) {
        Hardware.spi->beginTransaction(settings);
    }
}

void HardwareSPI::endTransaction(void){
    if (Hardware.spi != nullptr) {
        Hardware.spi->endTransaction();
    }
}

void HardwareSPI::attachInterrupt() {
    if (Hardware.spi != nullptr) {
        Hardware.spi->attachInterrupt();
    }
}

void HardwareSPI::detachInterrupt(){
    if (Hardware.spi != nullptr) {
        Hardware.spi->detachInterrupt();
    }
}

void HardwareSPI::begin() {
    if (Hardware.spi != nullptr) {
        Hardware.spi->begin();
    }
}

void HardwareSPI::end() {
    if (Hardware.spi != nullptr) {
        Hardware.spi->end();
    }
}
    
}