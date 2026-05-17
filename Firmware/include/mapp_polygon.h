#ifndef __MAPPING_POLYGON_H__
#define __MAPPING_POLYGON_H__

#include "mapping.h"

typedef struct touch_polygon_s touch_polygon_t;
struct touch_polygon_s {
  axis_t press;
  axis_t source[MAX_POLYGON_POINTS];
};

typedef struct polygon_s polygon_t;
struct polygon_s {
  uint8_t point_cnt;
  point_t point[MAX_POLYGON_POINTS];
  float m[MAX_POLYGON_POINTS];
  float c[MAX_POLYGON_POINTS];
  float max_dist;
  bool is_inside;
  uint8_t touchs;
  touch_polygon_t touch[MAX_POLYGON_TOUCHS];
  MidiType press;
  uint8_t input_chan;
};

bool mapping_polygons_alloc(uint8_t polygons_cnt);
void mapping_polygon_create(const JsonObject &config);

#endif /*__MAPPING_POLYGON_H__*/
