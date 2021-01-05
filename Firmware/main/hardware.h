/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "config.h"
#include "audio.h"

#include <Bounce2.h>                                    // https://github.com/thomasfredericks/Bounce2
//#include "../library/Bounce2/src/Bounce2.h"           // https://github.com/thomasfredericks/Bounce2 (FIXME)

#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>                                    // https://github.com/PaulStoffregen/Encoder
//#include "../library/Encoder/Encoder.h"               // https://github.com/PaulStoffregen/Encoder (FIXME)

//#include <EEPROM.h>                                   // https://www.arduino.cc/en/Reference/EEPROM
//#include "../library/..."                             // https://github.com/PaulStoffregen/Encoder (FIXME)

#include <elapsedMillis.h>                              // https://github.com/pfeerick/elapsedMillis
//#include "../library/elapsedMillis/elapsedMillis.h"   // https://github.com/pfeerick/elapsedMillis (FIXME)

typedef struct AudioControlSGTL5000 AudioControlSGTL5000_t; // Forward declaration

typedef struct preset {
  uint8_t mode;
  int8_t minVal;
  int8_t maxVal;
  int8_t val;
  int8_t lastVal;
  boolean toggle;
  boolean D1;
  boolean D2;
} preset_t;

void SETUP_SWITCHES(Button* buttonA_ptr, Button* buttonB_ptr);
void SETUP_LEDS(void);

void update_buttons(
  preset_t* preset_ptr,
  Button* buttonA_ptr,
  Button* buttonB_ptr,
  Encoder* encoder_ptr,
  uint8_t* mode_ptr,
  uint8_t* lastMode_ptr,
  uint8_t* iter_ptr,
  boolean* toggleSwitch_ptr,
  elapsedMillis* ledTimer_ptr
);

//
void update_preset(
  preset_t* preset_ptr,
  Encoder* encoder_ptr,
  boolean* calibrate_ptr,
  boolean* save_ptr,
  AudioControlSGTL5000* dac_ptr,
  elapsedMillis* timer_ptr
);

void update_leds(
  preset_t* preset_ptr,
  uint8_t* curentMode_ptr,
  uint8_t* lastMode_ptr,
  elapsedMillis* timer_ptr
);

void preset_load(preset_t* preset_ptr, boolean* state_ptr);
void preset_save(preset_t* preset_ptr, boolean* state_ptr);

boolean setLevel(preset_t* preset_ptr, Encoder* encoder_ptr);

#endif /*__HARDWARE_H__*/
