/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "player_granular.h"

void SETUP_GRANULAR(AudioEffectGranular* granular_ptr, uint16_t* buffer_ptr) {
  granular_ptr->begin(buffer_ptr, GRANULAR_BUFFER_SIZE);  // [ARGS](buffer_ptr, buffer_size)
}

void granular_player(llist_t* blobs_ptr, AudioEffectGranular* granular_ptr, uint16_t* buffer_ptr) {

  AudioNoInterrupts();
  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {

    if (blob_ptr->alive) {
      float playSpeed = blob_ptr->centroid.X / X_MAX; // Mapp X to buffer size
      float window = blob_ptr->centroid.Y / Y_MAX; // Mapp X to buffer size
      granular_ptr->setSpeed(playSpeed);
      granular_ptr->beginPitchShift(window);
    }
    else {
      granular_ptr->stop();
    }
  }
  AudioInterrupts();
}
