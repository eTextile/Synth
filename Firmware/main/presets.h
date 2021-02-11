/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PRESETS_H__
#define __PRESETS_H__

#include "config.h"

#include <Bounce2.h>                                    // https://github.com/thomasfredericks/Bounce2
//#include "../library/Bounce2/src/Bounce2.h"           // https://github.com/thomasfredericks/Bounce2 (FIXME)

#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>                                    // https://github.com/PaulStoffregen/Encoder
//#include "../library/Encoder/Encoder.h"               // https://github.com/PaulStoffregen/Encoder (FIXME)

//#include <EEPROM.h>                                   // https://www.arduino.cc/en/Reference/EEPROM (TODO)
//#include "../library/..."                             // (FIXME)

#include <elapsedMillis.h>                              // https://github.com/pfeerick/elapsedMillis
//#include "../library/elapsedMillis/elapsedMillis.h"   // https://github.com/pfeerick/elapsedMillis (FIXME)

typedef struct preset {
  uint8_t minVal;
  uint8_t maxVal;
  uint8_t val;
  uint8_t ledVal;
  boolean setLed;
  boolean updateLed;
  boolean update;
  boolean D1;
  boolean D2;
} preset_t;

void SETUP_SWITCHES(Button* buttonA_ptr, Button* buttonB_ptr);

void SETUP_LEDS(void);

void update_buttons(
  Button* buttonA_ptr,
  Button* buttonB_ptr,
  Encoder* encoder_ptr,
  preset_t* presets_ptr,
  uint8_t* curentMode_ptr,
  uint8_t* lastMode_ptr
);

void update_presets(
  preset_t* presets_ptr,
  Encoder* encoder_ptr,
  uint8_t* curentMode_ptr,
  uint8_t* interpThreshold_ptr
);

void update_leds(
  preset_t* preset_ptr,
  uint8_t* curentMode_ptr,
  uint8_t* lastMode_ptr
);

void preset_load(preset_t* preset_ptr, boolean* state_ptr); // TODO
void preset_save(preset_t* preset_ptr, boolean* state_ptr); // TODO

boolean setLevel(preset_t* preset_ptr, Encoder* encoder_ptr);

#endif /*__PRESETS_H__*/
