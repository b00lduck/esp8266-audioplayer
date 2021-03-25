/**
 * 
 * Copyright 2018-2021 D.Zerlett <daniel@zerlett.eu>
 * 
 * This file is part of esp32-audioplayer.
 * 
 * esp32-audioplayer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * esp32-audioplayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with esp32-audioplayer. If not, see <http://www.gnu.org/licenses/>.
 *  
 */
#pragma once
#include "Arduino.h"
#include "config.h"
#include <MFRC522.h>
#include "rfid/RC522_RFID_Utilities.h"

class NDEF {

  public:
    NDEF(MFRC522 *mfrc522);

    struct WifiConfig {
      char SSID[33];
      char password[64];      
    };

    bool readWifiConfig(WifiConfig *wifiConfig);    

  private:
    MFRC522 *mfrc522;
    RC522_RFID_Utilities rc522Utilities;
    bool readSector(byte* buffer, byte sector, byte block, byte numBlocks);   
};