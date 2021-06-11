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

#include "hardware_midi_transmit.h"
#include "usb_midi_transmit.h"
#include "usb_slip_osc_transmit.h"

typedef struct blob blob_t;         // Forward declaration
typedef struct llist llist_t;       // Forward declaration
typedef struct midiNode midiNode_t; // Forward declaration

extern llist_t blobs;

#undef round
#define round(x) lround(x)

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

void GRID_LAYOUT_SETUP(void);
void gridPopulate(void);
void gridPlay(void);
void trigger(tSwitch_t* switch_ptr);
void toggle(tSwitch_t* switch_ptr);
void hSlider(hSlider_t* slider_ptr);
void vSlider(vSlider_t* slider_ptr);
void cSlider(polar_t* polar_ptr, cSlider_t* slider_ptr);
void control_change(ccPesets_t* ccPesets_ptr);

#endif /*__MAPPING_H__*/
