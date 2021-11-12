/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "json_parser.h"

char json[1024] = {};

uint8_t map_triggers; 
uint8_t map_toggles;

void JSON_PARSER_SETUP(void) {

    //StaticJsonDocument<1024> doc;  // Stack
    DynamicJsonDocument doc(1024);   // Heap
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        digitalWrite(LED_PIN_D1, HIGH);
        digitalWrite(LED_PIN_D2, HIGH);
        delay(5);
        digitalWrite(LED_PIN_D1, LOW);
        digitalWrite(LED_PIN_D2, LOW);
        delay(5);
    } else {
        
        uint8_t map_triggers = doc["mapping"]["trigs"].size();
        mKey_t map_triggerParam[map_triggers];
        mSwitch_t map_triggerKey[map_triggers];

        for(uint8_t i =0; i < map_triggers; i++){
            map_triggerParam[i].posX = doc["mapping"]["trigs"][i][0];
            map_triggerParam[i].posY = doc["mapping"]["trigs"][i][1];
            map_triggerParam[i].size = doc["mapping"]["trigs"][i][2];
            map_triggerParam[i].note = doc["mapping"]["trigs"][i][3];
        };
        
        uint8_t map_toggles = doc["mapping"]["togs"].size();
        mKey_t map_toggleParam[map_toggles];
        mSwitch_t map_toggleKey[map_toggles];

        for(uint8_t i =0; i < map_toggles; i++){
            map_toggleParam[i].posX = doc["mapping"]["trigs"][i][0];
            map_toggleParam[i].posY = doc["mapping"]["trigs"][i][1];
            map_toggleParam[i].size = doc["mapping"]["trigs"][i][2];
            map_toggleParam[i].note = doc["mapping"]["trigs"][i][3];
        };
    };
};