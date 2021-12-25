/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __ALLOCATE_H__
#define __ALLOCATE_H__

#include <Arduino.h>

extern char* config_ptr;

char* allocate(char* data_ptr, unsigned int size);

#endif /*__ALLOCATE_H__*/