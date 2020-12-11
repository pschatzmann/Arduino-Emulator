#include "HardwareSPI.h"
#include "Service.h"
#include "Hardware.h"
/**
 * We suppport different implementations for the SPI
 *
 **/

namespace arduino {

//HardwareSPI::HardwareSPI() {}

//HardwareSPI::~HardwareSPI() {}

uint8_t HardwareSPI::transfer(uint8_t data) {
    return Hardware.spi->transfer(data);
}

uint16_t HardwareSPI::transfer16(uint16_t data) {
    return Hardware.spi->transfer16(data);
}

void HardwareSPI::transfer(void *data, size_t count) {
    Hardware.spi->transfer(data, count);
}

void HardwareSPI::usingInterrupt(int interruptNumber) {
    Hardware.spi->usingInterrupt(interruptNumber);
}

void HardwareSPI::notUsingInterrupt(int interruptNumber){
    Hardware.spi->notUsingInterrupt(interruptNumber);
}

void HardwareSPI::beginTransaction(SPISettings settings){
    Hardware.spi->beginTransaction(settings);
}

void HardwareSPI::endTransaction(void){
    Hardware.spi->endTransaction();
}

void HardwareSPI::attachInterrupt() {
    Hardware.spi->attachInterrupt();
}

void HardwareSPI::detachInterrupt(){
    Hardware.spi->detachInterrupt();
}

void HardwareSPI::begin() {
    Hardware.spi->begin();
}

void HardwareSPI::end() {
    Hardware.spi->end();
}
    
}