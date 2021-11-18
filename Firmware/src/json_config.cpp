/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "json_config.h"
<<<<<<< HEAD
/*
=======
#include "mapping_lib.h"

#include <ArduinoJson.h>
#include <SerialFlash.h>

>>>>>>> 102d795054ed0b09a2ecc421375aba8ec3e758a0
#define FLASH_CHIP_SELECT  6

inline bool config_load_mapping_triggers(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  }
  mapping_triggers_alloc(config.size());
  for (uint8_t keyPos = 0; keyPos < map_trigs; keyPos++) {
    map_trigParams[keyPos].posX = config[keyPos]["posX"];
    map_trigParams[keyPos].posY = config[keyPos]["posY"];
    map_trigParams[keyPos].size = config[keyPos]["size"];
    map_trigParams[keyPos].note = config[keyPos]["note"];
  }
  return true;
}

inline bool config_load_mapping_toggles(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  }
  mapping_toggles_alloc(config.size());
  for (uint8_t keyPos = 0; keyPos < map_togs; keyPos++) {
    map_togParams[keyPos].posX = config[keyPos]["posX"];
    map_togParams[keyPos].posY = config[keyPos]["posY"];
    map_togParams[keyPos].size = config[keyPos]["size"];
    map_togParams[keyPos].note = config[keyPos]["note"];
  }
  return true;
}

inline bool config_load_mapping(const JsonObject& config) {
  if (config.isNull()) {
    return false;
  }

  if (!config_load_mapping_triggers(config["triggers"])) {
    return false;
  }

  if (!config_load_mapping_toggles(config["toggles"])) {
    return false;
  }

  return true;
}

static void config_error(void) {
  while (1) {
    digitalWrite(LED_PIN_D1, HIGH);
    digitalWrite(LED_PIN_D2, HIGH);
    delay(5);
    digitalWrite(LED_PIN_D1, LOW);
    digitalWrite(LED_PIN_D2, LOW);
    delay(5);
  };
}

void LOAD_SPI_FLASH_CONFIG() {

  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    config_error();
  }
<<<<<<< HEAD
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
=======

  uint8_t configData[1024];
  SerialFlashFile configFile = SerialFlash.open("config.json");
  configFile.read(configData, configFile.size());

  StaticJsonDocument<1024> config;
  DeserializationError error = deserializeJson(config, configData);
  if (error) {
    config_error();
  }

  if (!config_load_mapping(config["mapping"])) {
    // FIXME: loading JSON config failed! abort? alert? panic?
  }

  configFile.close();
}
>>>>>>> 102d795054ed0b09a2ecc421375aba8ec3e758a0
