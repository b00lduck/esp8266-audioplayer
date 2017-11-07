/**
 * 
 * Copyright 2017 D.Zerlett <daniel@zerlett.eu>
 * 
 * This file is part of esp8266-audioplayer.
 * 
 * esp8266-audioplayer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * esp8266-audioplayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with esp8266-audioplayer. If not, see <http://www.gnu.org/licenses/>.
 *  
 */
#include <Wire.h>
#include "csmux.h"

CSMultiplexer::CSMultiplexer(uint8_t _i2cAddress) : 
  i2cAddress(_i2cAddress), 
  selectedSpiAddress(255) {}

void CSMultiplexer::init() {
 
  // Configure ports of the MCP32017 

  // Set PORTA data direction register
  Wire.beginTransmission(i2cAddress);
  Wire.write(0x00); // IODIRA 
  Wire.write(0x00); // PORTA output
  Wire.endTransmission();

  // PORTB unused for now
  //Wire.beginTransmission(0x20);
  //Wire.write(0x01); // IODIRB
  //Wire.write(0x00); // PORTB output
  //Wire.endTransmission();  

  // Set PORTA to 0xff (all chips deselected)
  Wire.beginTransmission(i2cAddress);
  Wire.write(0x12); // PORTA
  Wire.write(0xff); // value
  Wire.endTransmission();    
  
  // PORTB unused for now
  //Wire.beginTransmission(0x20);
  //Wire.write(0x13); // PORTB
  //Wire.write(0b11000000); // value
  //Wire.endTransmission();    
}

/**
 * Select external SPI chip by pulling one of the outputs of PORTA to logic LOW
 */
void CSMultiplexer::chipSelect(uint8_t spiAddress) {    
  if (selectedSpiAddress != spiAddress) {
    uint8_t value = 0xff - (1 << spiAddress);    
    Wire.beginTransmission(i2cAddress);
    Wire.write(0x12);
    Wire.write(value);
    Wire.endTransmission();    
    selectedSpiAddress = spiAddress;
  }
}

/**
 * Deselect all external SPI chips by setting all CS lines to logic HIGH
 */
void CSMultiplexer::chipDeselect() {
    Wire.beginTransmission(i2cAddress);
    Wire.write(0x12);
    Wire.write(0xff);
    Wire.endTransmission();
    selectedSpiAddress = 255;
}

