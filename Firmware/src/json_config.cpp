/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "json_config.h"
#include "mapping_lib.h"

#include <ArduinoJson.h>
#include <SerialFlash.h>

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
    digitalWrite(LED_BUILTIN, HIGH);
    delay(5);
    digitalWrite(LED_BUILTIN, LOW);
    delay(5);
  };
}

void LOAD_SPI_FLASH_CONFIG() {

  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    config_error();
  }

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
