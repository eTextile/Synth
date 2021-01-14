/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "config.h"
#include "blob.h"
#include "presets.h"

#define   PI 3.1415926535897932384626433832795
#define   MAPP_DEBOUNCE_TIME  500

typedef struct blob blob_t;   // Forward declaration
typedef struct blob blob_t;   // Forward declaration

typedef struct {
  float r;
  float phi;
} polar_t;

typedef struct {
  uint8_t posX;
  uint8_t posY;
} keyCode_t;

typedef struct {
  uint8_t posX;
  uint8_t posY;
  uint8_t rSize; // width/2 and height/2
  elapsedMillis debounceTimer;
  boolean state;
} switch_t;

boolean toggle(blob_t* blob_ptr, switch_t* tSwitch);
boolean trigger(blob_t* blob_ptr, switch_t* tSwitch);

keyCode_t gridLayout(blob_t* blob_ptr, uint8_t gridW, uint8_t gridH, uint8_t stepX, uint8_t stepY, uint8_t posX, uint8_t posY);

void harmonicKeyboardLayout(blob_t* blob_ptr);

void velocity(blob_t* blob_ptr);

void hSlider(blob_t* blob_ptr, uint8_t posY, uint8_t Xmin, uint8_t Xmax, uint8_t height);
void vSlider(blob_t* blob_ptr, uint8_t posX, uint8_t Ymin, uint8_t Ymax, uint8_t width);

polar_t polarCoordinates(blob_t* blob_ptr, uint8_t Xcenter, uint8_t Ycenter);
void cSlidercSlider(polar_t blob, float radius, float tetaMin, float tetaMax, uint8_t width);

#endif /*__MAPPING_H__*/
