/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "config.h"
#include "blob.h"
#include "llist.h"
#include "notes.h"
#include "midi_transmit.h"

//typedef struct midiMsg midiMsg_t;   // Forward declaration
//typedef struct midiNode midiNode_t; // Forward declaration

typedef struct key stroke_t;
struct key {
  uint8_t posX;
  uint8_t posY;
  uint8_t size;
  uint8_t note;
};

typedef struct rect rect_t;
struct rect {
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t Ymin;
  uint8_t Ymax;
};
typedef struct tSwitch tSwitch_t;
struct tSwitch {
  rect_t rect;
  boolean state;
};

typedef struct vSlider vSlider_t;
struct vSlider {
  uint8_t posX;
  uint8_t Ymin;
  uint8_t Ymax;
  uint8_t width;
  int8_t val;
};

typedef struct hSlider hSlider_t;
struct hSlider {
  uint8_t posY;
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t height;
  int8_t val;
};

typedef struct polar polar_t;
struct polar {
  float radius;
  float theta;
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
  int8_t val;
};

typedef struct cChange cChange_t;
struct cChange {
  uint8_t blobID;
  uint8_t mappVal;
  int8_t cChange;
  int8_t midiChannel;
  int8_t lastVal;
};

void GRID_LAYOUT_SETUP(void);

void CSLIDERS_SETUP(void);
void TRIGGERS_SETUP(void);
void TOGGLES_SETUP(void);
void VSLIDERS_SETUP(void);
void HSLIDERS_SETUP(void);
void CCHANGE_SETUP(void);

void mapping_grid_populate(void);
void mapping_grid_update(void);
void mapping_grid_play(void); // TODO

void mapping_cSlider(void);

void mapping_trigger(void);
void mapping_toggle(void);

void mapping_vSlider(void);
void mapping_hSlider(void);
void mapping_cChange(void);

#endif /*__MAPPING_H__*/
