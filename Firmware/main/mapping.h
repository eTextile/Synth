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

typedef struct llist llist_t;         // Forward declaration
typedef struct blob blob_t;           // Forward declaration
typedef struct midiNode midiNode_t;   // Forward declaration
typedef struct cChange cChange_t;     // Forward declaration

//extern llist_t blobs;
//extern llist_t midiIn;
//extern llist_t midiOut;

#undef round
#define round(x) lround(x)

#define GRID_COLS   14
#define GRID_ROWS   10
#define GRID_KEYS   (GRID_COLS * GRID_ROWS)
#define GRID_GAP    1
#define KEY_SIZE_X  (uint8_t)((X_MAX - (GRID_GAP * (GRID_COLS + 1))) / GRID_COLS)
#define KEY_SIZE_Y  (uint8_t)((Y_MAX - (GRID_GAP * (GRID_ROWS + 1))) / GRID_ROWS)

#define C_SLIDERS  3

typedef struct squareKey squareKey_t;
struct squareKey {
  int8_t val;
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t Ymin;
  uint8_t Ymax;
};

typedef struct tSwitch tSwitch_t;
struct tSwitch {
  uint8_t posX;
  uint8_t posY;
  uint8_t rSize; // width/2 and height/2
  uint32_t timeStamp;
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

typedef struct cSlider cSlider_t;
struct cSlider {
  float r;
  uint8_t width;
  float phiOffset;
  float phiMin;
  float phiMax;
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
void mapping_trigger(tSwitch_t* switch_ptr);
void mapping_toggle(tSwitch_t* switch_ptr);
void mapping_hSlider(hSlider_t* slider_ptr);
void mapping_vSlider(vSlider_t* slider_ptr);
void mapping_cSliders(cSlider_t* slider_ptr);
void mapping_cChange(cChange_t* cChange_ptr);

#endif /*__MAPPING_H__*/
