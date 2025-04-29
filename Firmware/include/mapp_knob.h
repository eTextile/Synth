#ifndef __MAPPING_KNOB_H__
#define __MAPPING_KNOB_H__

#include "mapping.h"

typedef struct knob_touch_s knob_touch_t;
struct knob_touch_s {
  msg_t radius;
  uint8_t last_midi_radius;
  msg_t theta;
  uint8_t last_midi_theta;
  msg_t press;
  uint8_t last_midi_press;
  msg_t note;
  //uint8_t last_midi_note; // NOT_USED!
};

typedef struct knob_s knob_t;
struct knob_s {
  rect_t rect;
  point_t center;
  float radius;
  float offset;
  uint8_t touchs;
  knob_touch_t touch[MAX_KNOB_TOUCHS];
  MidiType mode_z;
};

bool mapping_knobs_alloc(uint8_t knobs_cnt);
void mapping_knob_create(const JsonObject &config);

#endif /*__MAPPING_KNOB_H__*/
