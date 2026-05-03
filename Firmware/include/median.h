/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MEDIAN_H__
#define __MEDIAN_H__

#include "config.h"
#include "llist.h"
#include "blob.h"

// EMA smoothing factor for blob centroid position.
// 0 = frozen (infinite lag), 1 = raw (no smoothing).
// At 600 fps a value of 0.3 gives ~2-frame lag with good noise rejection.
#define EMA_ALPHA_POSITION 0.3f

typedef struct median median_t;
struct median {
  float x; // EMA state for centroid.x
  float y; // EMA state for centroid.y
};

void running_median_setup(void);
void runing_median(void);

#endif /*__MEDIAN_H__*/
