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

// MODES
typedef enum {
  LINE_OUT,
  SIG_IN,
  SIG_OUT,
  THRESHOLD,
  MIDI_LEARN,
  CALIBRATE,
  SAVE
} presetMode_t;

typedef struct preset preset_t;
struct preset {
  uint8_t minVal;
  uint8_t maxVal;
  uint8_t val;
  uint8_t ledVal;
  boolean setLed;
  boolean updateLed;
  boolean update;
  boolean D1;
  boolean D2;
};

void SWITCHES_SETUP(Button* buttonA_ptr, Button* buttonB_ptr);

void LEDS_SETUP(void);

void update_buttons(
  Button* buttonA_ptr,
  Button* buttonB_ptr,
  Encoder* encoder_ptr,
  preset_t* presets_ptr,
  presetMode_t* curentMode_ptr,
  presetMode_t* lastMode_ptr
);

void update_presets(
  presetMode_t curentMode,
  preset_t* presets_ptr,
  Encoder* encoder_ptr,
  uint8_t* interpThreshold_ptr
);

void update_leds(
  presetMode_t curentMode,
  preset_t* preset_ptr
);

void preset_load(preset_t* preset_ptr, boolean* state_ptr); // TODO
void preset_save(preset_t* preset_ptr, boolean* state_ptr); // TODO

boolean setLevel(preset_t* preset_ptr, Encoder* encoder_ptr);

#endif /*__PRESETS_H__*/
