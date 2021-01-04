/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "config.h"
#include "llist.h"
#include "blob.h"

typedef struct llist llist_t; // Forward declaration
typedef struct blob blob_t;   // Forward declaration

void KeyboardGrid(llist_t* blobs_ptr);
void KeyboardHarmonic(llist_t* blobs_ptr);

void velocity(llist_t* blobs_ptr);

void hSlider(void);
void vSlider(void);
void cSlider(void);

void toggle(void);
void trigger(void);


#endif /*__MAPPING_H__*/
