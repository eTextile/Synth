/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "config.h"
#include "blob.h"

#define   PI 3.1415926535897932384626433832795

typedef struct blob blob_t;   // Forward declaration

typedef struct {
  float r;
  float phi;
} polar_t;

typedef struct {
  uint8_t posX;
  uint8_t posY;
} keyCode_t;

polar_t polarCoordinates(blob_t* blob_ptr);

keyCode_t gridLayout(blob_t* blob_ptr, uint8_t posX, uint8_t posY, uint8_t gridW, uint8_t gridH, uint8_t stepX, uint8_t stepY );

void harmonicKeyboardLayout(blob_t* blob_ptr);

void velocity(blob_t* blob_ptr);

void hSlider(void);
void vSlider(void);
void cSlider(void);

void toggle(void);
void trigger(void);


#endif /*__MAPPING_H__*/
