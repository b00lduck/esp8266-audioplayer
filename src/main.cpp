/**
 * 
 * Copyright 2018-2020 D.Zerlett <daniel@zerlett.eu>
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

#include <stdio.h>
#include <string.h>
#include <Wire.h>
#include <NeoPixelBus.h>
#ifdef OTA_ENABLED
  #include <ArduinoOTA.h>
#endif

#include "config.h"
#include "buttons.h"
#include "player/VS1053.h"
#include "player/player.h"
#include "storage/sd.h"
#include "storage/mapper.h"
#include "http/http.h"

#include "rfid/ndef.h"
#include "fatal.h"

VS1053    vs1053(VS1053_XCS_PIN, VS1053_XDCS_PIN, VS1053_DREQ_PIN, VS1053_XRESET_PIN);
RFID      rfid(MFRC522_CS_PIN, MFRC522_RST_PIN);
SDCard    sd(SD_CS_PIN);

Fatal     fatal;
Player    player(fatal, vs1053);

Mapper    mapper;
Buttons   buttons;
HTTP      http(&rfid, &mapper, &sd);

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixels(4, WS2812_DATA_PIN);

void setup() {
 
  Serial.begin(115200);                            
  Serial.println("MP3 player V2 now Booting...");

  pixels.Begin();
  pixels.ClearTo(RgbColor(10,0,10));
  pixels.Show();

  //http.init();

  pixels.ClearTo(RgbColor(0,10,10));
  pixels.Show();

  // Set shutdown pin to input
  pinMode(SHUTDOWN_PIN, INPUT);
  
  // Initialize GPIOs for "amplifier enable" and shut it down
  pinMode(AMP_ENABLE_PIN, OUTPUT);
  digitalWrite(AMP_ENABLE_PIN, LOW); 

  // Initialize buttons
  buttons.init();

  // Initialize GPIO for battery voltage
  pinMode(ADC_BATT, ANALOG);

  // Initialize SPI bus
  SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
  Serial.println("[ OK ] SPI init completed");

  // Initialize RFID reader
  rfid.init();
  Serial.println("[ OK ] RFID init completed");
 
  // Initialize SD card reader
  if (!sd.init()) {
    pixels.ClearTo(RgbColor(0,30,0));
    pixels.Show();
    fatal.fatal("SD card error", "init failed");
  }
  Serial.println("[ OK ] SD/MMC init completed");

  Mapper::MapperError err = mapper.init(); 
  if (err != Mapper::MapperError::OK) {
    pixels.ClearTo(RgbColor(0,30,0));
    pixels.Show();
    switch(err) {
      default:
        break;
    }    
  }

  // initialize player
  player.init();
  Serial.println("[ OK ] MP3 init completed");
  pixels.ClearTo(RgbColor(0,10,0));
  pixels.Show();   

  player.playlist.addEntry("/system/startup.mp3");
  player.play();

  #ifdef OTA_ENABLED
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
          type = "sketch";
        } else {
          // U_SPIFFS
          type = "filesystem";
        }
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("[OTA] Start updating firware");
      })
      .onEnd([]() {
        Serial.println("\n[OTA] End");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("[OTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
      });

    ArduinoOTA.begin();  
  #endif
}

uint16_t lpf = 0;
uint16_t showBatt = 5000;

enum MAIN_MODE {
  PLAYER,
  ADMIN
};

MAIN_MODE mode = PLAYER;





void loop() {

  #ifdef OTA_ENABLED
    ArduinoOTA.handle();  
  #endif

  bool changed = buttons.read();

  // Cheapo one pole IIR low pass filter with unknown cutoff frequency (because sample rate is unknown).
  // This is good enough for battery monitoring though.
  //
  // lpf:
  // 0 is 0V battery voltage  
  // 1030 is approx. 4.3V battery voltage
  // in between linear (voltage divider)
  uint16_t batt = analogRead(ADC_BATT);
  uint16_t newLpf = (float)lpf * 0.90 + (float)batt * 0.1;
  lpf = newLpf;

  // ADC Reference: 4096 = 3300mV
  // Divider: 4.2V batt -> 980mV

  showBatt++;
  if (showBatt > 2000) {
    showBatt = 0;
    Serial.printf("[BATT] %1.2fV (%d)\n", lpf/ADC_DIVISOR, lpf);
    Serial.printf("[PLYR] idle time %d\n", player.idleTime);
    Serial.printf("[HEAP] %d\n", ESP.getFreeHeap());
  }

  NDEF::WifiConfig wifiConfig;

  // Read card
  RFID::CardState cardState = rfid.checkCardState(&wifiConfig);
  
  switch(mode) {

    case PLAYER:

      pixels.ClearTo(RgbColor(0,30,0));
      pixels.Show();

      if (player.idleTime > IDLE_SHUTDOWN_FADE_START) {
          if (player.idleTime > IDLE_SHUTDOWN_TIMEOUT) { //  shutdown timer
            Serial.println("[HALT] idle shutdown");
            pinMode(SHUTDOWN_PIN, OUTPUT);
            digitalWrite(SHUTDOWN_PIN, HIGH);
            sleep(30);
          }
        }
      
        // Volume Control
        //if (buttons.buttonDown(4)) {
        //  player.increaseVolume();
       // }

        //if (buttons.buttonDown(0)) {
        //  player.decreaseVolume();
        //}        

        // Track Control
        if (changed) {
          if (buttons.buttonDown(3)) {
            player.next();
          } else if (buttons.buttonDown(1)) {
            player.previous();
          }        
        }
              
        switch(cardState) {
          case RFID::CardState::NEW_MEDIA_CARD:
            
            char currentCardString[CARD_ID_STRING_LENGTH];
            rfid.currentCardAsString(currentCardString);
            mapper.createPlaylist(&player.playlist, currentCardString);
            player.play();

            break;
            //{
              /*
              char filename[MAX_FILENAME_STRING_LENGTH];
              Mapper::MapperError err = mapper.resolveIdToFilename(rfid.currentCard, filename);    
              switch(err) {
                case Mapper::MapperError::CARD_ID_NOT_FOUND:
                  Serial.println(F("Card not found in mapping"));
                  player.stop();
                  break;     
                case OK:
                  player.play(filename);
                  break;
                default:
                  fatal.fatal("Mapping error", "Unknown error");
                  break;
              } 
            */
            //}

          case RFID::CardState::NEW_WIFI_CARD:
            player.stop(true);

            pixels.ClearTo(RgbColor(30,0,0));
            pixels.Show();

            if (http.start(&wifiConfig)) {
              mode = ADMIN;
            }
            break;
            
          case RFID::CardState::REMOVED_CARD:            
            player.stop(true);
            break;
          case RFID::CardState::FAULTY_CARD:
            player.stop(true);
            break;
          case RFID::CardState::NO_CHANGE:
            break;
        }

        player.process();
      break;

    case ADMIN:
      pixels.ClearTo(RgbColor(30,30,0));
      pixels.Show();
      if (changed) {
        http.shutdown();
        mode = PLAYER;
      }
      break;

  }

}
