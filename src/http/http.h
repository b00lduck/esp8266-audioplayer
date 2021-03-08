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
#include "Arduino.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "rfid.h"
#include "config.h"
#include "../storage/sd.h"
#include "../storage/mapper.h"

#define MAX_UPLOAD_PATH_LEN 300

class HTTP {

  public: 
    HTTP(RFID *rfid, Mapper *mapper, SDCard *sd);
    void init();   

  private:
    AsyncWebServer server;

    RFID   *rfid;
    Mapper *mapper;
    SDCard *sd;

    #ifdef ENABLE_CORS
      void addCorsHeader(AsyncWebServerResponse *response);
      void handlerCors(AsyncWebServerRequest *request);
    #endif 
    
    void handlerCurrentCardGet(AsyncWebServerRequest *request);  
    void handlerCardGet(AsyncWebServerRequest *request);    
    void handlerCardPost(AsyncWebServerRequest *request);                             
    void handlerCardPut(AsyncWebServerRequest *request);   

    void handlerFileGet(AsyncWebServerRequest *request);
    void handlerFileDelete(AsyncWebServerRequest *request);
    void handlerFilePost(AsyncWebServerRequest *request);
    void handlerFilePostUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    
    bool uploadInProgress;
    char uploadPath[MAX_UPLOAD_PATH_LEN]; // "/cards/12341234/<max 255>" 1+5+1+8+255+1=271
    File uploadFile;

};
