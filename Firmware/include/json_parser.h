/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__

#include "config.h"
#include "mapping.h"

#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson.git

extern char json[];

typedef struct mKey mKey_t;         // Forward declaration located in blob.h
typedef struct mSwitch mSwitch_t;   // Forward declaration located in blob.h

extern uint8_t map_triggers; 
extern mKey_t map_triggerParam[];
extern mSwitch_t map_triggerKey[];

extern uint8_t map_toggles; 
extern mKey_t map_toggleParam[];
extern mSwitch_t map_toggleKey[];

void JSON_PARSER_SETUP(void);

#endif /*__JSON_PARSER__*/