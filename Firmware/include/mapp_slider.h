#ifndef __MAPPING_SLIDER_H__
#define __MAPPING_SLIDER_H__

#include "mapping.h"

typedef enum dir_e {
  VERTICAL,
  HORIZONTAL
} dir_t;

typedef struct slider_s slider_t;
struct slider_s {
  rect_t rect;
  dir_t dir;
  uint8_t touchs;
  touch_linear_t touch[MAX_SLIDER_TOUCHS];
  MidiType press;
  move_t move;
  populate_t populate;
  uint8_t steps;
  uint8_t step_note[MAX_SLIDER_STEPS];
  uint8_t input_chan;
};

bool mapping_sliders_alloc(uint8_t sliders_cnt);
void mapping_slider_create(const JsonObject &config);

#endif /*__MAPPING_SLIDER_H__*/
