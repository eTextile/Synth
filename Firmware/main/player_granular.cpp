/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "player_granular.h"

#define GRANULAR_MEMORY_SIZE 12800  // enough for 290 ms at 44.1 kHz
int16_t granularMemory[GRANULAR_MEMORY_SIZE] = {0};

void GRANULAR_PLAYER_SETUP(AudioEffectGranular* granular_ptr) {
  granular_ptr->begin(granularMemory, GRANULAR_MEMORY_SIZE);  // [ARGS](buffer_ptr, buffer_size)
}

void granular_player(llist_t* llist_ptr, AudioEffectGranular* granular_ptr) {
  blob_t* lastBlob_ptr = (blob_t*)llist_ptr->tail_ptr;
  if (lastBlob_ptr->state) {
    if (!lastBlob_ptr->lastState) {
      float msec = lastBlob_ptr->centroid.Y / Y_MAX; // Mapp X to buffer size
      msec = round(10.0 + msec * 10.0);
      granular_ptr->beginPitchShift(msec); // The grainLength is specified in milliseconds, up to 1/3 of the memory from begin();
    }
    else {
      float ratio = lastBlob_ptr->centroid.X / X_MAX; // Mapp X to buffer size
      ratio = powf(2.0, ratio * 2.0 - 1.0); // 0.5 to 2.0
      //ratio = powf(2.0, ratio * 6.0 - 3.0); // 0.125 to 8.0 -- uncomment for far too much range!
      granular_ptr->setSpeed(ratio);
    }
  }
  else {
    granular_ptr->stop();
  }
}
