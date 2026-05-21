#ifndef __MAPPING_SWITCH_H__
#define __MAPPING_SWITCH_H__

#include "mapping.h"

typedef struct switch_s switch_t;
struct switch_s {
  rect_t rect;
  uint8_t touchs;
  touch_press_t touch[MAX_SWITCH_TOUCHS];
  midi_chord_t chord[MAX_SWITCH_TOUCHS]; // only used when press == MIDI_TYPE_CHORD
  MidiType press;
  uint8_t input_chan;
  bool tap_tempo; // true = tap tempo mode: each touch sends MIDI TimingClock at tapped BPM
};

bool mapping_switchs_alloc(uint8_t switchs_cnt);
void mapping_switch_create(const JsonObject &config);

#endif /*__MAPPING_SWITCH_H__*/
