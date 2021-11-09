/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "player_granular.h"

#if defined(GRANULAR_PLAYER)
AudioInputI2S           i2s_IN;
AudioOutputI2S          i2s_OUT;
AudioEffectGranular     granular;
AudioConnection         patchCord1(i2s_IN, 0, granular, 0);
AudioConnection         patchCord2(granular, 0, i2s_OUT, 0);
AudioConnection         patchCord3(granular, 0, i2s_OUT, 1);

#define GRANULAR_MEMORY_SIZE 12800  // Enough for 290 ms at 44.1 kHz
int16_t granularMemory[GRANULAR_MEMORY_SIZE] = {0};

void GRANULAR_PLAYER_SETUP(void) {
  granular.begin(&granularMemory[0], GRANULAR_MEMORY_SIZE);  // [ARGS](buffer_ptr, buffer_size)
};

void granular_player(void) {

  blob_t* blob_ptr = (blob_t*)blobs.tail_ptr;
  if (blob_ptr != NULL) {
    AudioNoInterrupts();
    if (blob_ptr->state) {
      if (!blob_ptr->lastState) {
        float msec = blob_ptr->centroid.Y; // Blob_Y mapped to grainLength
        msec = round(10 + msec);
        granular.beginPitchShift(msec);    // The grainLength is specified in milliseconds, up to 1/3 of the memory from begin();
      }
      else {
        float ratio = blob_ptr->centroid.X / (float)X_MAX; // Mapp X to buffer size
        ratio = powf(2.0, ratio * 2.0 - 1.0);              // 0.5 to 2.0
        //ratio = powf(2.0, ratio * 6.0 - 3.0);            // 0.125 to 8.0 -- uncomment for far too much range!
        granular.setSpeed(ratio);
      }
      AudioInterrupts();
    }
    else {
      granular.stop();
    };
  };
};
#endif