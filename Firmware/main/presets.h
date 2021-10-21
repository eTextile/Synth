/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PRESETS_H__
#define __PRESETS_H__

#include "config.h"
#include "interp.h"
#include "llist.h"
#include "midi_transmit.h"

#include <Bounce2.h>                   // https://github.com/thomasfredericks/Bounce2
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>                   // https://github.com/PaulStoffregen/Encoder
//#include <EEPROM.h>                  // https://www.arduino.cc/en/Reference/EEPROM (TODO)

#define LINE_OUT         0 // LED | 0 | 0 |
#define SIG_IN           1 // LED | 1 | 0 |
#define SIG_OUT          2 // LED | 0 | 1 |
#define THRESHOLD        3 // LED | 1 | 1 |
#define CALIBRATE        4
#define SAVE             5
#define BLOBS_PLAY       6
#define BLOBS_LEARN      7
#define BLOBS_MAPPING    8
#define RAW_MATRIX       9
#define INTERP_MATRIX    10
#define ALL_OFF          11

/*
  typedef enum synthMode {
  LINE_OUT,          // [0]
  SIG_IN,            // [1]
  SIG_OUT,           // [2]
  THRESHOLD,         // [3]
  CALIBRATE,         // [4]
  SAVE,              // [5]
  MIDI_BLOBS_PLAY,   // [6]
  MIDI_BLOBS_LEARN,  // [7]
  MIDI_MAPPING,      // [8]
  MIDI_RAW,          // [9]
  MIDI_INTERP,       // [10]
  MIDI_OFF,          // [11]
  } synthMode_t;
*/

extern Encoder encoder;       // Exposed local declaration see presets.cpp
extern uint8_t currentMode;   // Exposed local declaration see presets.cpp
extern uint8_t lastMode;      // Exposed local declaration see presets.cpp

typedef struct preset preset_t;
struct preset {
  uint8_t minVal;
  uint8_t maxVal;
  uint8_t val;
  uint8_t ledVal;
  boolean update;
  boolean setLed;
  boolean updateLed;
  boolean allDone;
  boolean D1;
  boolean D2;
};

extern preset_t presets[];        // Exposed local declaration see presets.cpp

void LEDS_SETUP(void);
void SWITCHES_SETUP(void);

void update_presets_midi_usb(void);
void update_presets_buttons(void);
void update_presets_encoder(void);
void update_leds(void);

boolean setLevel(preset_t* preset_ptr);

void load_presets(void); // TODO
void save_presets(void); // TODO

#endif /*__PRESETS_H__*/
