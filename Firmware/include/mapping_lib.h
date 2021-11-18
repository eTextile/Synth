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

#define MAX_VERTICES 5

typedef struct center center_t;
struct center {
    float x;
    float y;
};

typedef struct circle circle_t;
struct circle {
    center_t c;
    float r;
    float offset;
};

typedef struct vertice vertice_t;
struct vertice {
    float x;
    float y;
};

typedef struct polygon polygon_t;
struct polygon {
    uint8_t vertices_cnt;
    float m[MAX_VERTICES]; // slope of the line
    float c[MAX_VERTICES];
    vertice_t vertices[MAX_VERTICES];
};

typedef struct mKey mKey_t;
struct mKey {
  uint8_t posX;
  uint8_t posY;
  uint8_t size;
  uint8_t note;
};

typedef struct rect rect_t;
struct rect {
  float Xmin;
  float Xmax;
  float Ymin;
  float Ymax;
};

typedef struct mSwitch mSwitch_t;
struct mSwitch {
  rect_t rect;
  boolean state;
};

typedef struct vSlider vSlider_t;
struct vSlider {
  uint8_t posX;
  uint8_t Ymin;
  uint8_t Ymax;
  uint8_t width;
  uint8_t cChange;
  int8_t lastVal;
};

typedef struct hSlider hSlider_t;
struct hSlider {
  uint8_t posY;
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t height;
  uint8_t cChange;
  int8_t lastVal;
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

typedef struct cChange cChange_t;
struct cChange {
  uint8_t blobID;
  uint8_t mappVal;
  uint8_t cChange;
  uint8_t lastX, lastY, lastZ, lastW, lastH;
};

void MAPPING_LIB_SETUP(void);
void mapping_lib_update(void);

void MAPPING_GRID_SETUP(void);
void MAPPING_POLYGONS_SETUP(void);
void MAPPING_TOUCHPADS_SETUP(void);
void MAPPING_CIRCLES_SETUP(void);
void MAPPING_TRIGGERS_SETUP(void);
void MAPPING_TOGGLES_SETUP(void);
void MAPPING_VSLIDERS_SETUP(void);
void MAPPING_HSLIDERS_SETUP(void);
void MAPPING_CSLIDERS_SETUP(void);

void mapping_grid_updete(void);
void mapping_grid_populate(void);
void mapping_polygons_update(void);
void mapping_touchpads_updete(void);
void mapping_circles_updete(void);
void mapping_triggers_updete(void);
void mapping_toggles_updete(void);
void mapping_vSliders_updete(void);
void mapping_hSliders_updete(void);
void mapping_cSliders_updete(void);
void mapping_blobs_updete(void);

#endif /*__MAPPING_LIB_H__*/