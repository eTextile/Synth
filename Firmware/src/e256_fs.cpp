/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "e256_fs.h"

LittleFS_SPIFlash myfs;

const int chipSelect = 6;

File file;

char json[1024] = {};

void FS_SETUP(void) {

  if (!myfs.begin(chipSelect, SPI)) {
      while (1){
          digitalWrite(LED_PIN_D1, HIGH);
          digitalWrite(LED_PIN_D2, HIGH);
          delay(100);
          digitalWrite(LED_PIN_D1, LOW);
          digitalWrite(LED_PIN_D2, LOW);
          delay(100);
      }
    };
  
  if (myfs.exists("/config.json")){
      file = myfs.open("/config.json", FILE_READ);
      file.read(json, 1024);
      file.close();
    } else {
        return;
    };

    //StaticJsonDocument<1024> doc;  // Stack
    DynamicJsonDocument doc(1024);   // Heap

    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    } else {
        /*
        stroke_t triggerParam[json["trigs"]];
        for(int i =0; i<(uint8_t)json["trigs"]; i++){
            triggerParam[i].posX = json["trig"][posX];
            triggerParam[i].posY = json["trig"][posY];
            triggerParam[i].size = json["trig"][size];
            triggerParam[i].note = json["trig"][note];
        };

        stroke_t toggleParam[(uint8_t)json["togs"]];
        for(int i =0; i<(uint8_t)json["togs"]; i++){
            triggerParam[i].posX = json["togs"][posX];
            triggerParam[i].posY = json["togs"][posY];
            triggerParam[i].size = json["togs"][size];
            triggerParam[i].note = json["togs"][note];
        };
        */
    }
};