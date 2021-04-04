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
#include "transmit.h"

typedef struct blob blob_t;         // Forward declaration
typedef struct llist llist_t;       // Forward declaration
typedef struct midiNode midiNode_t; // Forward declaration

typedef struct squareKey squareKey_t;
struct squareKey {
  float Xmin;
  float Xmax;
  float Ymin;
  float Ymax;
};

typedef struct tSwitch tSwitch_t;
struct tSwitch {
  uint8_t posX;
  uint8_t posY;
  uint8_t rSize; // width/2 and height/2
  uint32_t timer;
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

typedef struct grid grid_t;
struct grid {
  int8_t* lastKey;
  squareKey_t* keyArray_ptr;
  llist_t* midiIn;
};

void GRID_LAYOUT_SETUP(squareKey_t* keyArray_ptr);

void gridLayout(llist_t* llist_ptr, grid_t* gridLayout_ptr);
void gridGapLayout(llist_t* llist_ptr, grid_t* gridLayout_ptr);
void hSlider(llist_t* llist_ptr, hSlider_t* slider_ptr);
void vSlider(llist_t* llist_ptr, vSlider_t* slider_ptr);
void cSlider(llist_t* llist_ptr, polar_t* polar_ptr, cSlider_t* slider_ptr);
boolean toggle(llist_t* llist_ptr, tSwitch_t* switch_ptr);
boolean trigger(llist_t* llist_ptr, tSwitch_t* switch_ptr);

/*
  typedef struct seq seq_t;
  struct seq {
  uint16_t timeInterval[];
  uint8_t* seqframe;
  };

  void tapTempo(tSwitch_t* tSwitch_ptr, tapTempo_t* tapTempo_ptr); // TODO
  void seq(tSwitch_t* tSwitch_ptr, seq_t* seq_ptr); // TODO
  void arpeggiator(llist_t* notes_ptr); // TODO
  void stepSequencer(void); // TODO?
*/

#endif /*__MAPPING_H__*/
