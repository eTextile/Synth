/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAPPING_LIB_H__
#define __MAPPING_LIB_H__

#include "config.h"
#include "blob.h"
#include "llist.h"
#include "notes.h"
#include "midi_bus.h"

typedef struct center center_t;
struct center {
    float x;
    float y;
};

typedef struct circle circle_t;
struct circle {
    center_t center;
    float radius;
    float offset;
    float CCr;
    float CCt;
    //float lastRadius;
    //float lasttheta;
};

typedef struct point point_t;
struct point {
    float x;
    float y;
};

typedef struct rect rect_t;
struct rect {
  point_t from;
  point_t to;
};

typedef struct key keysroke_t;
struct key {
  rect_t rect;
  uint8_t chan;
  uint8_t note;
  uint8_t velocity;
  boolean state;
};


typedef struct slider slider_t;
struct slider {
  rect_t rect;
  boolean dir; // H_SLIDER || V_SLIDER
  uint8_t chan;
  uint8_t cc;
  uint8_t min;
  uint8_t max;
  uint8_t val;
};

typedef struct cTrack cTrack_t;
struct cTrack {
  uint8_t sliders;
  uint8_t index;
  float offset;
};

typedef struct cSlider cSlider_t;
struct cSlider {
  uint8_t id;
  float thetaMin;
  float thetaMax;
  float lastVal;
};

typedef struct polygon polygon_t;
struct polygon {
    uint8_t point_cnt;
    point_t point[MAX_POLYGON_POINT];
    float m[MAX_POLYGON_POINT];
    float c[MAX_POLYGON_POINT];
};
typedef struct e256_touch e256_touch_t;
struct e256_touch {
  uint8_t Xchan;
  uint8_t Xcc;
  uint8_t Ychan;
  uint8_t Ycc;
  uint8_t Zchan;
  uint8_t Zcc;
};

typedef struct touchpad touchpad_t;
struct touchpad {
  rect_t rect;
  uint8_t touchs; // Max blobs
  e256_touch_t touch[MAX_TOUCH_POINT];
  uint8_t min;    // 
  uint8_t max;    //
};

extern uint8_t mapp_trigs;
extern keysroke_t *mapp_trigsParams;
void mapping_triggers_alloc(uint8_t count);

extern uint8_t mapp_switchs;
extern keysroke_t *mapp_switchParams;
void mapping_switchs_alloc(uint8_t count);

extern uint8_t mapp_sliders;
extern slider_t *mapp_slidersParams;
void mapping_sliders_alloc(uint8_t count);

extern uint8_t mapp_circles;
extern circle_t *mapp_circlesParams;
void mapping_circles_alloc(uint8_t count);

extern uint8_t mapp_polygons;
extern polygon_t *mapp_polygonsParams;
void mapping_polygons_alloc(uint8_t count);

extern uint8_t mapp_touchpads;
extern touchpad_t *mapp_touchpadsParams;
void mapping_touchpads_alloc(uint8_t count);

void mapping_lib_setup(void);

void mapping_triggers_setup(void);
void mapping_switchs_setup(void);
void mapping_sliders_setup(void);
void mapping_circles_setup(void);
void mapping_touchpads_setup(void);
void mapping_polygons_setup(void);
void mapping_grids_setup(void);

void mapping_tirggers_updete(void);
void mapping_switchs_updete(void);
void mapping_sliders_updete(void);
void mapping_circles_updete(void);
void mapping_polygons_update(void);
void mapping_touchpads_updete(void);

void mapping_grid_populate(void);
void mapping_grid_updete(void);

void mapping_lib_update(void);

#endif /*__MAPPING_LIB_H__*/
