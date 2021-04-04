/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "player_granular.h"

void GRANULAR_PLAYER_SETUP(AudioEffectGranular* granular_ptr, uint16_t* buffer_ptr) {
  granular_ptr->begin(buffer_ptr, GRANULAR_MEMORY_SIZE);  // [ARGS](buffer_ptr, buffer_size)
}

void granular_player(llist_t* blobs_ptr, AudioEffectGranular* granular_ptr, uint16_t* buffer_ptr) {

  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {

    if (blob_ptr->state && !blob_ptr->lastState) {
    }

    if (blob_ptr->state) {
      float msec = blob_ptr->centroid.Y / Y_MAX; // Mapp X to buffer size
      msec = 10.0 + (msec * 10.0);
      granular_ptr->beginPitchShift(msec); // The grainLength is specified in milliseconds, up to 1/3 of the memory from begin();
      float ratio = blob_ptr->centroid.X / X_MAX; // Mapp X to buffer size
      ratio = powf(2.0, ratio * 2.0 - 1.0); // 0.5 to 2.0
      //ratio = powf(2.0, ratio * 6.0 - 3.0); // 0.125 to 8.0 -- uncomment for far too much range!
      granular_ptr->setSpeed(ratio);
    }
    else {
      granular_ptr->stop();
    }
  }
}
