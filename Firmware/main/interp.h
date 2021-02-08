/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __INTERP_H__
#define __INTERP_H__

#include "config.h"
#include "blob.h"

typedef struct image image_t; // forward declaration

#undef round
#define round(x) lround(x)

typedef struct interp {
  uint8_t   scale_X;
  uint8_t   scale_Y;
  uint16_t  outputStride_Y;
  float*    pCoefA;
  float*    pCoefB;
  float*    pCoefC;
  float*    pCoefD;
} interp_t;

void SETUP_INTERP(
  image_t*  inputFrame_ptr,
  uint8_t*  inputArray_ptr,
  image_t*  outputFrame,
  uint8_t*  outputArray,
  interp_t* interp
);

void interp_matrix(
  uint8_t   interpThreshold,
  image_t*  outputFrame_ptr,
  image_t*  inputFrame_ptr,
  interp_t* interp_ptr
);

void print_interp(image_t* array_ptr);

#endif /*__INTERP_H__*/
