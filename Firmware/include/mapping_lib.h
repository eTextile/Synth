/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAPPING_LIB_H__
#define __MAPPING_LIB_H__

#include "config.h"
#include "blob.h"
#include "llist.h"
#include "notes.h"
#include "json_config.h"
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
    //float lastRadius;
    //float lasttheta;
    //uint8_t CCradius;
    //uint8_t CCtheta;
};

typedef struct vertice vertice_t;
struct vertice {
    float x;
    float y;
};

typedef struct polygon polygon_t;
struct polygon {
    uint8_t vertices_cnt;
    vertice_t vertices[MAX_VERTICES];
    float m[MAX_VERTICES];
    float c[MAX_VERTICES];
};

typedef struct rect rect_t;
struct rect {
  float Xmin;
  float Xmax;
  float Ymin;
  float Ymax;
};

typedef struct Key Key_t;
struct Key {
  rect_t rect;
  boolean state;
  uint8_t note;
};

typedef struct touchpad touchpad_t;
struct touchpad {
  rect_t rect;
  uint8_t CCx;    // X axis MIDI cChange mapping
  uint8_t CCy;    // X axis MIDI cChange mapping
  uint8_t CCz;    // X axis MIDI cChange mapping
  uint8_t CCs;    // XY size MIDI cChange mapping
  uint8_t CCxyv;  // XY velocity MIDI cChange mapping
  uint8_t CCzv;   // XY velocity MIDI cChange mapping
};

typedef struct vSlider vSlider_t;
struct vSlider {
  rect_t rect;
  uint8_t CC;
};

typedef struct hSlider hSlider_t;
struct hSlider {
  rect_t rect;
  uint8_t CC;
};

typedef struct cTrack cTrack_t;
struct cTrack {
  uint8_t sliders;
  uint8_t index;
  float offset;
};

typedef struct cSlider cSlider_t;
struct cSlider {
  float thetaMin;
  float thetaMax;
  float lastVal;
};

extern uint8_t mapp_touchpads;
extern touchpad_t *mapp_touchpadsParams;
void mapping_touchpads_alloc(uint8_t count);

extern uint8_t mapp_trigs;
extern Key_t *mapp_trigsParams;
void mapping_triggers_alloc(uint8_t count);

extern uint8_t mapp_togs;
extern Key_t *mapp_togsParams;
void mapping_toggles_alloc(uint8_t count);

extern uint8_t mapp_circles;
extern circle_t *mapp_circlesParams;
void mapping_circles_alloc(uint8_t count);

extern uint8_t mapp_polygons;
extern polygon_t *mapp_polygonsParams;
void mapping_polygons_alloc(uint8_t count);

extern uint8_t mapp_hSliders;
extern hSlider_t *mapp_hSlidersParams;
void mapping_hSliders_alloc(uint8_t count);

extern uint8_t mapp_vSliders;
extern vSlider_t *mapp_vSlidersParams;
void mapping_vSliders_alloc(uint8_t count);

void MAPPING_LIB_SETUP(void);

void MAPPING_TOGGLES_SETUP(void);
void MAPPING_TRIGGERS_SETUP(void);
void MAPPING_VSLIDERS_SETUP(void);
void MAPPING_HSLIDERS_SETUP(void);
void MAPPING_CIRCLES_SETUP(void);
void MAPPING_TOUCHPADS_SETUP(void);
void MAPPING_POLYGONS_SETUP(void);
void MAPPING_GRID_SETUP(void);
void MAPPING_CSLIDERS_SETUP(void);

void mapping_toggles_updete(void);
void mapping_tirggers_updete(void);
void mapping_vSliders_updete(void);
void mapping_hSliders_updete(void);
void mapping_circles_updete(void);
void mapping_touchpads_updete(void);
void mapping_polygons_update(void);
void mapping_grid_populate(void);
void mapping_grid_updete(void);
void mapping_cSliders_updete(void);

void mapping_lib_update(void);

#endif /*__MAPPING_LIB_H__*/