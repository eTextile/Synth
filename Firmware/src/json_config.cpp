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
  for (uint8_t i = 0; i < map_trigs; i++) {
    map_trigsParams[i].posX = config[i]["posX"];
    map_trigsParams[i].posY = config[i]["posY"];
    map_trigsParams[i].size = config[i]["size"];
    map_trigsParams[i].note = config[i]["note"];
  }
  return true;
}

inline bool config_load_mapping_toggles(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  }
  mapping_toggles_alloc(config.size());
  for (uint8_t i = 0; i < map_togs; i++) {
    map_togsParams[i].posX = config[i]["posX"];
    map_togsParams[i].posY = config[i]["posY"];
    map_togsParams[i].size = config[i]["size"];
    map_togsParams[i].note = config[i]["note"];
  }
  return true;
}

inline bool config_load_mapping_circles(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  }
  
  mapping_circles_alloc(config.size());
  for (uint8_t i = 0; i < map_circles; i++) {
    map_circlesParams[i].center.x = config[i]["centerX"];
    map_circlesParams[i].center.y = config[i]["centerY"];
    map_circlesParams[i].radius = config[i]["radius"];
    map_circlesParams[i].offset = config[i]["offset"];
  };
  return true;
}

inline bool config_load_mapping(const JsonObject& config) {
  if (config.isNull()) {
    return false;
  };

  if (!config_load_mapping_triggers(config["triggers"])) {
    return false;
  };

  if (!config_load_mapping_toggles(config["toggles"])) {
    return false;
  };

  if (!config_load_mapping_triggers(config["circles"])) {
    return false;
  };

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
