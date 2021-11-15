/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "json_parser.h"

void JSON_PARSER_SETUP(void) {

  //StaticJsonDocument<1024> doc;  // Stack
  DynamicJsonDocument doc(1024);   // Heap

  DeserializationError error = deserializeJson(doc, jsonFile);

  if (error) {
    while (1) {
      digitalWrite(LED_PIN_D1, HIGH);
      digitalWrite(LED_PIN_D2, HIGH);
      delay(5);
      digitalWrite(LED_PIN_D1, LOW);
      digitalWrite(LED_PIN_D2, LOW);
      delay(5);
    };
  };
};