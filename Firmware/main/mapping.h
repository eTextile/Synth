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
#include "transmit.h"

#undef round
#define round(x) lround(x)

typedef struct llist llist_t;         // Forward declaration
typedef struct blob blob_t;           // Forward declaration
typedef struct midiNode midiNode_t;   // Forward declaration
typedef struct cChange cChange_t;     // Forward declaration

extern llist_t blobs;
extern llist_t midiIn;
extern llist_t midiOut;

typedef struct rect rect_t;
struct rect {
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t Ymin;
  uint8_t Ymax;
};

typedef struct tSwitch tSwitch_t;
struct tSwitch {
  uint8_t posX;
  uint8_t posY;
  uint8_t size;
  boolean state;
};

typedef struct vSlider vSlider_t;
struct vSlider {
  uint8_t posX;
  uint8_t Ymin;
  uint8_t Ymax;
  uint8_t width;
  uint8_t val;
};

typedef struct hSlider hSlider_t;
struct hSlider {
  uint8_t posY;
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t height;
  uint8_t val;
};

typedef struct cTrack cTrack_t;
struct cTrack {
  uint8_t div;
  float rMin;
  float rMax;
};

typedef struct cSlider cSlider_t;
struct cSlider {
  float phiMin;
  float phiMax;
  float phiOffset;
  uint8_t val;
};

typedef struct cChange cChange_t;
struct cChange {
  uint8_t blobID;
  uint8_t mappVal;
  int8_t cChange;
  int8_t midiChannel;
  int8_t val;
};

void GRID_LAYOUT_SETUP(void);
void mapping_gridPopulate(void);
void mapping_gridPlay(void);

void C_SLIDERS_SETUP(void);
void mapping_cSliders(void);

void TRIGGERS_SETUP(void);
tSwitch_t* mapping_triggers(void);

void TOGGLES_SETUP(void);
tSwitch_t* mapping_toggles(void);

void VSLIDERS_SETUP(void);
void mapping_vSliders(void);

void HSLIDERS_SETUP(void);
void mapping_hSliders(void);

void mapping_cChange(cChange_t* cChange_ptr);

#endif /*__MAPPING_H__*/
