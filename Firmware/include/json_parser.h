/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__

#include "config.h"
#include "mtp_spi.h"

#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson.git
//using namespace ArduinoJson6185_91;

extern DynamicJsonDocument doc();

void JSON_PARSER_SETUP(void);

#endif /*__JSON_PARSER__*/