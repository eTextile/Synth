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

typedef struct blob blob_t;    // Forward declaration
typedef struct llist llist_t;  // Forward declaration

#if MIDI_HARDWARE
#include <MIDI.h>
#endif

void SETUP_MIDI_HARDWARE(void);

typedef struct velocity velocity_t;

struct velocity {
  float lastX;
  float lastY;
  float lastZ;
  float vx;
  float vy;
  float vz;
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

typedef struct polar polar_t;

struct polar {
  float r;
  float phi;
};

typedef struct squareKey squareKey_t;

struct squareKey {
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t Ymin;
  uint8_t Ymax;
};

typedef struct grid grid_t;

struct grid {
  squareKey_t* keyArray_ptr;
  int8_t* keyPressed;
  int8_t* midiLayout;
};

typedef struct cChange ccPesets_t;

struct cChange {
  uint8_t blobID;
  uint8_t mappVal;
  int8_t cChange;
  int8_t midiChannel;
  int8_t val;
};

typedef struct seq seq_t;

struct seq {
  uint16_t timeInterval[];
  uint8_t* seqframe;
};

void SETUP_GRID_LAYOUT(squareKey_t* keyArray_ptr);

void gridLayout(llist_t* blobs_ptr, grid_t* gridLayout_ptr);
void gridGapLayout(llist_t* blobs_ptr, grid_t* gridLayout_ptr);

//uint16_t dist(blob_t* blob_ptr, grid_t* grid_ptr, uint16_t* keyIndexArray_ptr);

void controlChangeMapping(llist_t* blobs_ptr, ccPesets_t* pesets_ptr);
void hSlider(llist_t* blobs_ptr, hSlider_t* slider_ptr);
void vSlider(llist_t* blobs_ptr, vSlider_t* slider_ptr);
void cSlider(llist_t* blobs_ptr, polar_t* polar_ptr, cSlider_t* slider_ptr);
boolean toggle(llist_t* blobs_ptr, tSwitch_t* switch_ptr);
boolean trigger(llist_t* blobs_ptr, tSwitch_t* switch_ptr);

void getPolarCoordinates(llist_t* blobs_ptr, polar_t* polarPos_ptr);
void getVelocity(llist_t* blobs_ptr, velocity_t* velocity_ptr);

//void tapTempo(tSwitch_t* tSwitch_ptr, tapTempo_t* tapTempo_ptr); // TODO
//void seq(tSwitch_t* tSwitch_ptr, seq_t* seq_ptr); // TODO
//void arpeggiator(llist_t* notes_ptr); // TODO
//void stepSequencer(void); // TODO?

#endif /*__MAPPING_H__*/
