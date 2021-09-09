/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PRESETS_H__
#define __PRESETS_H__

#include "config.h"
#include "interp.h"

#include <Bounce2.h>                   // https://github.com/thomasfredericks/Bounce2
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>                   // https://github.com/PaulStoffregen/Encoder
//#include <EEPROM.h>                  // https://www.arduino.cc/en/Reference/EEPROM (TODO)

#define LINE_OUT        0
#define SIG_IN          1
#define SIG_OUT         2
#define THRESHOLD       3
#define CALIBRATE       4
#define SAVE            5

#define MIDI_RAW        6
#define MIDI_INTERP     7
#define MIDI_LEARN      8
#define MIDI_BLOBS      9
#define MIDI_OFF        10

typedef struct interp interp_t;        // forward declaration

extern uint8_t interpThreshold;
extern uint8_t currentMode;
extern uint8_t lastMode;

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

extern preset_t presets[];

void LEDS_SETUP(void);
void SWITCHES_SETUP(void);
void update_buttons(void);
void update_presets(void);
void update_leds(void);
boolean setLevel(preset_t* preset_ptr);

void load_presets(void); // TODO
void save_presets(void); // TODO

#endif /*__PRESETS_H__*/
