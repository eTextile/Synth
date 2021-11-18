/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "json_config.h"
/*
#define FLASH_CHIP_SELECT  6

uint8_t map_trigs = 0;
mKey_t map_trigParams[] = {0};
mSwitch_t map_trigKeys[] = {0};

uint8_t map_togs = 0;
mKey_t map_togParams[] = {0};
mSwitch_t map_togKeys[] = {0};


void LOAD_SPI_FLASH_CONFIG() {

  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    while (1) {
      digitalWrite(LED_PIN_D1, HIGH);
      digitalWrite(LED_PIN_D2, HIGH);
      delay(5);
      digitalWrite(LED_PIN_D1, LOW);
      digitalWrite(LED_PIN_D2, LOW);
      delay(5);
    };
  }
  else {

    uint8_t buffer[1024];
    SerialFlashFile conf = SerialFlash.open("config.json");
    uint32_t size = conf.size();
    conf.read(buffer, size);

    //DynamicJsonDocument jsonFile(conf.size());
    DynamicJsonDocument jsonFile(1024);
    DeserializationError error = deserializeJson(jsonFile, conf["mapp"]);
    
    if (error) {
      while (1) {
        digitalWrite(LED_PIN_D1, HIGH);
        digitalWrite(LED_PIN_D2, HIGH);
        delay(5);
        digitalWrite(LED_PIN_D1, LOW);
        digitalWrite(LED_PIN_D2, LOW);
        delay(5);
      };
    }
    else {
      map_trigs = jsonFile["mapp"]["trigs"].size();
      map_trigParams[map_trigs];
      map_trigKeys[map_trigs];
      for (uint8_t i = 0; i < map_trigs; i++) {
        map_trigParams[i].posX = jsonFile["mapp"]["trigs"][i][0];
        map_trigParams[i].posY = jsonFile["mapp"]["trigs"][i][1];
        map_trigParams[i].size = jsonFile["mapp"]["trigs"][i][2];
        map_trigParams[i].note = jsonFile["mapp"]["trigs"][i][3];
      };

      map_togs = jsonFile["mapp"]["togs"].size();
      map_togParams[map_togs];
      map_togKeys[map_togs];
      for (uint8_t i = 0; i < map_togs; i++) {
        map_togParams[i].posX = jsonFile["mapp"]["togs"][i][0];
        map_togParams[i].posY = jsonFile["mapp"]["togs"][i][1];
        map_togParams[i].size = jsonFile["mapp"]["togs"][i][2];
        map_togParams[i].note = jsonFile["mapp"]["togs"][i][3];
      };
      conf.close();
    };
  };
};
*/