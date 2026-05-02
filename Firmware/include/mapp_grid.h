#ifndef __MAPPING_GRID_H__
#define __MAPPING_GRID_H__

#include "mapping.h"

typedef struct grid_s grid_t;
struct grid_s {
  rect_t rect;
  uint8_t touchs;
  touch_1d_t touch[MAX_GRID_TOUCHS];
  uint8_t keys;
  touch_1d_t key[MAX_GRID_KEYS];
  uint8_t cols;
  uint8_t rows;
  MidiType press;
  uint8_t input_chan;
  float scale_factor_x;
  float scale_factor_y;
};

bool mapping_grids_alloc(uint8_t touchpads_cnt);
void mapping_grid_create(const JsonObject &config);

#endif /*__MAPPING_GRID_H__*/