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

#if MIDI_HARDWARE
#include <MIDI.h>
#endif

#define   PI 3.1415926535897932384626433832795
#define   DEBOUNCE_SWITCH_TIME  80
#define   GRID_STEPS_X 16
#define   GRID_STEPS_Y 16

typedef struct blob blob_t;    // Forward declaration
typedef struct llist llist_t;  // Forward declaration

void MIDI_SETUP(void);

typedef struct {
  float r;
  float phi;
} polar_t;

typedef struct {
  uint8_t index;
  float windowX[VELOCITY_WINDOW];
  float windowY[VELOCITY_WINDOW];
  float windowZ[VELOCITY_WINDOW];
  float velocityX;
  float velocityY;
  float velocityZ;
} velocity_t;

typedef struct {
  uint8_t posX;
  uint8_t posY;
  uint8_t rSize; // width/2 and height/2
  unsigned long debounce;
  boolean state;
} switch_t;

typedef struct {
  uint16_t keyPos;
  uint16_t lastKeyPos;
  unsigned long debounce;
} keyPos_t;

typedef struct {
  uint8_t posX;
  uint8_t Ymin;
  uint8_t Ymax;
  uint8_t width;
  uint8_t val;
} sliderV_t;

typedef struct {
  uint8_t posY;
  uint8_t Xmin;
  uint8_t Xmax;
  uint8_t height;
  uint8_t val;
} sliderH_t;

void gridLayoutSet(
  //TODO
);

void gridLayoutPlay(
  llist_t* blobs_ptr,
  keyPos_t* key_ptr,
  uint8_t posX,
  uint8_t posY,
  uint8_t gridW,
  uint8_t gridH
);

uint16_t harmonicKeyboardLayout(
  llist_t* blobs_ptr,
  keyPos_t* key_ptr,
  uint8_t posX,
  uint8_t posY,
  uint8_t gridW,
  uint8_t gridH
);

void hSlider(llist_t* blobs_ptr, sliderH_t* slider_ptr);

void vSlider(llist_t* blobs_ptr, sliderV_t* slider_ptr);

void cSlidercSlider(polar_t blob, float radius, float tetaMin, float tetaMax, uint8_t wSize);

void getPolarCoordinates(llist_t* blobs_ptr, polar_t* polarPos_ptr);

void getVelocity(llist_t* blobs_ptr, velocity_t* velocity_ptr);

boolean toggle(llist_t* blobs_ptr, switch_t* tSwitch);

boolean trigger(llist_t* blobs_ptr, switch_t* tSwitch);

#endif /*__MAPPING_H__*/
