/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__

#include "config.h"
#include "mapping_lib.h"

#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson.git

extern char json[];

typedef struct mKey mKey_t;         // Forward declaration located in blob.h
typedef struct mSwitch mSwitch_t;   // Forward declaration located in blob.h

extern uint8_t map_trigs; 
extern mKey_t map_trigParams[];
extern mSwitch_t map_trigKeys[];

extern uint8_t map_togs; 
extern mKey_t map_togParams[];
extern mSwitch_t map_togKeys[];

void JSON_PARSER_SETUP(void);

#endif /*__JSON_PARSER__*/