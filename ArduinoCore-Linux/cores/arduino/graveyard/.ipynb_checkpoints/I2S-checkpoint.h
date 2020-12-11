/*
  Copyright (c) 2016 Arduino LLC.  All right reserved.
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _I2S_H_
#define _I2S_H_

#include <Arduino.h>

typedef enum {
  I2S_PHILIPS_MODE,
  I2S_RIGHT_JUSTIFIED_MODE,
  I2S_LEFT_JUSTIFIED_MODE
} i2s_mode_t;

class I2SClass : public Stream
{
public:
  // the device index and pins must map to the "COM" pads in Table 6-1 of the datasheet 
  I2SClass(uint8_t deviceIndex, uint8_t clockGenerator, uint8_t sdPin, uint8_t sckPin, uint8_t fsPin){
      create(deviceIndex,clockGenerator,sdPin,sckPin,fsPin );
  }
  
  void create(uint8_t deviceIndex, uint8_t clockGenerator, uint8_t sdPin, uint8_t sckPin, uint8_t fsPin);

  // the SCK and FS pins are driven as outputs using the sample rate
  int begin(int mode, long sampleRate, int bitsPerSample);
  // the SCK and FS pins are inputs, other side controls sample rate
  int begin(int mode, int bitsPerSample);
  void end();

  // from Stream
  virtual int available();
  virtual int read();
  virtual int peek();
  virtual void flush();

  // from Print
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buffer, size_t size);

  virtual int availableForWrite();

  int read(void* buffer, size_t size);

//  size_t write(int);
  size_t write(int32_t);
  size_t write(const void *buffer, size_t size);

//  void onTransmit(void(*)(void));
//  void onReceive(void(*)(void));

  void setBufferSize(int bufferSize);

  


};

extern I2SClass I2S;

#endif
