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
  
extern uint8_t interpThreshold;
extern image_t rawFrame;

extern image_t interpFrame;

void INTERP_SETUP(void);
void interp_matrix(void);

#endif /*__INTERP_H__*/
