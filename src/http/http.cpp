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
#include "http.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

HTTP::HTTP(RFID *rfid, Mapper *mapper, SDCard *sd) : 
  server(80),
  rfid(rfid),
  mapper(mapper),
  sd(sd)
{} 

void notFound(AsyncWebServerRequest *request) {
    request->send(404, F("text/plain"), F("Not found"));
}

#ifdef ENABLE_CORS
void HTTP::addCorsHeader(AsyncWebServerResponse *response) {
  response->addHeader(F("Access-Control-Allow-Origin"),"*");
  response->addHeader(F("Access-Control-Allow-Methods"),"*");
  response->addHeader(F("Access-Control-Allow-Headers"),"*");
}

void HTTP::handlerCors(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(204);
  #ifdef ENABLE_CORS
    addCorsHeader(response);
  #endif
  request->send(response);
}
#endif

bool HTTP::start(NDEF::WifiConfig *wifiConfig) {  

  WiFi.persistent(false);
  WiFi.disconnect(true, true);

  uint8_t triesLeft = 30;

  // try connection to WiFi
  WiFi.begin(wifiConfig->SSID, wifiConfig->password);
  Serial.printf("Connection to %s with secret %s\n", wifiConfig->SSID, wifiConfig->password);
  while ((WiFi.status() != WL_CONNECTED) && (triesLeft)){
    delay(1000);
    Serial.print(F("."));
    triesLeft--;
  }  

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Could not connect to WiFi"));
    return false;
  }

  Serial.println(WiFi.localIP());

  #ifdef ENABLE_CORS
    server.on("*", HTTP_OPTIONS, std::bind(&HTTP::handlerCors, this, std::placeholders::_1));
  #endif


  // create server and routes
  const char currentCardUrl[] = "/api/card/current"; 
  server.on(currentCardUrl, HTTP_GET, std::bind(&HTTP::handlerCurrentCardGet, this, std::placeholders::_1));

  const char cardWithIdUrl[] = "^\\/api\\/card\\/([0-9a-fA-F]{8})$";
  server.on(cardWithIdUrl, HTTP_POST, std::bind(&HTTP::handlerCardPost, this, std::placeholders::_1));
  server.on(cardWithIdUrl, HTTP_PUT, std::bind(&HTTP::handlerCardPut, this, std::placeholders::_1));

  const char cardUrl[] = "/api/card";
  server.on(cardUrl, HTTP_GET, std::bind(&HTTP::handlerCardGet, this, std::placeholders::_1));

  const char fileUrl[] = "^\\/api\\/file(\\/.*)*$";
  server.on(fileUrl, HTTP_GET, std::bind(&HTTP::handlerFileGet, this, std::placeholders::_1));

  const char fileDeleteUrl[] = "^\\/api\\/file(\\/.*)$";
  server.on(fileDeleteUrl, HTTP_DELETE, std::bind(&HTTP::handlerFileDelete, this, std::placeholders::_1));

  const char fileUploadUrl[] = "^\\/api\\/file\\/(.*)$";
  server.on(fileUploadUrl, HTTP_POST, 
    std::bind(&HTTP::handlerFilePost, this, std::placeholders::_1),
    std::bind(&HTTP::handlerFilePostUpload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)
  );

  server.onNotFound(notFound);
  server.begin(); 

  return true;
}

void HTTP::shutdown() {  
  server.end();
  WiFi.persistent(false);
  WiFi.disconnect(true, true);
  Serial.println(F("Stopped HTTP server and disconnected WiFi"));
}
