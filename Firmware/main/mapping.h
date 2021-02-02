/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
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

typedef struct velocity {
  float lastValX;
  float lastValY;
  float lastValD;
  float velocityX;
  float velocityY;
  float velocityD;
} velocity_t;

typedef struct tSwitch {
  uint8_t posX;
  uint8_t posY;
  uint8_t rSize; // width/2 and height/2
  unsigned long timer;
  boolean state;
} tSwitch_t;

typedef struct vSlider {
  uint8_t posX;
  uint8_t Ymin;
  uint8_t Ymax;
  uint8_t width;
  uint8_t val;
  uint8_t lastVal;
} vSlider_t;

typedef struct hSlider {
  uint8_t posY;
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t height;
  uint8_t val;
  uint8_t lastVal;
} hSlider_t;

typedef struct cSlider {
  float r;
  uint8_t width;
  float phiOffset;
  float phiMin;
  float phiMax;
  uint8_t val;
  uint8_t lastVal;
} cSlider_t;

typedef struct polar {
  float r;
  float phi;
} polar_t;

typedef struct gridPoint {
  float X;
  float Y;
} gridPoint_t;

typedef struct cChange {
  uint8_t blobID;
  blob_t* blobVal;
  uint8_t lastVal;
  uint8_t cchnage;
  uint8_t midiChannel;
  unsigned long timer;
} ccPesets_t;

typedef struct grid {
  gridPoint_t* keyPosArray;
  uint16_t lastWinPos[MAX_BLOBS];
  int16_t lastKeyIndex[MAX_BLOBS];
  unsigned long timer[MAX_BLOBS];
  uint8_t* midiNotes;
} grid_t;

// TODO

void SETUP_KEYBOARD_LAYOUT(gridPoint_t* keyPosArray_ptr);

void ControlChangeMapping(llist_t* blobs_ptr, ccPesets_t* pesets_ptr);

void gridSetLayout(grid_t* grid_ptr);

void gridLayoutMapping_A(llist_t* blobs_ptr, grid_t* grid_ptr);

uint16_t dist(blob_t* blob_ptr, grid_t* grid_ptr, uint16_t* keyIndexArray_ptr);
void gridLayoutMapping_B(llist_t* blobs_ptr, grid_t* grid_ptr);

void hSlider(llist_t* blobs_ptr, hSlider_t* slider_ptr);

void vSlider(llist_t* blobs_ptr, vSlider_t* slider_ptr);

void cSlider(llist_t* blobs_ptr, polar_t* polar_ptr, cSlider_t* slider_ptr);

void getPolarCoordinates(llist_t* blobs_ptr, polar_t* polarPos_ptr);

boolean toggle(llist_t* blobs_ptr, tSwitch_t* switch_ptr);

boolean trigger(llist_t* blobs_ptr, tSwitch_t* switch_ptr);

void getVelocity(llist_t* blobs_ptr, velocity_t* velocity_ptr);

#endif /*__MAPPING_H__*/
