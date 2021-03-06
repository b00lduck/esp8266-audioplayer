/**
 * 
 * Copyright 2018 D.Zerlett <daniel@zerlett.eu>
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
#include "rfid.h"
#include "tools.h"

RFID::RFID(uint8_t _csPin, uint8_t _rstPin) : 
  csPin(_csPin),
  rstPin(_rstPin),
  mfrc522(csPin,rstPin)
  {}

void RFID::init() {
   mfrc522.PCD_Init();  
   mfrc522.PCD_DumpVersionToSerial(); 
  // mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
}

RFID::CardState RFID::checkCardState() {
  
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      cardFailCount = 0;    
      if (cardChanged(mfrc522.uid.uidByte, mfrc522.uid.size)) {
        newCard(mfrc522.uid.uidByte, mfrc522.uid.size);        
        return CardState::NEW_CARD;        
      }      
    } else {
        Serial.println(F("Error reading card"));
        cardPresent = false;   
        return CardState::FAULTY_CARD;                   
    }
  } else {
    if (cardPresent) {
      cardFailCount++;
      if (cardFailCount > 1) {
          Serial.println(F("Card removed"));
          cardPresent = false;   
          memset(currentCard, 0, sizeof(currentCard));
          return CardState::REMOVED_CARD;                 
      }     
    }
  } 

   return CardState::NO_CHANGE;
}

/** 
 * compares buffer with the currently active card  
 * return true if card ID has changed
 */
bool RFID::cardChanged(byte *buffer, byte bufferSize) {
  for (uint8_t i = 0; i < bufferSize; i++) {
    if (buffer[i] != currentCard[i]) {
      return true;
    }
  }
  return false;
}

/**
 * called when a new card is detected
 */
void RFID::newCard(byte *buffer, byte bufferSize) {
  cardPresent = true;
  if (bufferSize > sizeof(currentCard)) {
    bufferSize = sizeof(currentCard);
  }
  memcpy(currentCard, buffer, bufferSize);
  Serial.print(F("New Card with UID"));
  dumpByteArray(mfrc522.uid.uidByte, mfrc522.uid.size);  
  Serial.println(F(" detected."));  
}
