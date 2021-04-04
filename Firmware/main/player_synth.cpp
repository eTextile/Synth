/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "player_synth.h"

void SYNTH_PLAYER_SETUP(synth_t* allSynth_ptr) {
  AudioNoInterrupts();
  for (int i = 0; i < 8; i++) {
    allSynth_ptr[i].fade_ptr->fadeOut(0);
    allSynth_ptr[i].wf_ptr->begin(WAVEFORM_SINE);
    allSynth_ptr[i].wf_ptr->amplitude(0.9);
    allSynth_ptr[i].fm_ptr->amplitude(0.9);
  }
  allSynth_ptr[0].mix_ptr->gain(0, 0.25);
  allSynth_ptr[0].mix_ptr->gain(1, 0.25);
  allSynth_ptr[0].mix_ptr->gain(2, 0.25);
  allSynth_ptr[0].mix_ptr->gain(3, 0.25);

  allSynth_ptr[4].mix_ptr->gain(0, 0.25);
  allSynth_ptr[4].mix_ptr->gain(1, 0.25);
  allSynth_ptr[4].mix_ptr->gain(2, 0.25);
  allSynth_ptr[4].mix_ptr->gain(3, 0.25);
  AudioInterrupts();
}

/////////////////////////// MAKE NOISE FONCTION !
void synth_player(llist_t* blobs_ptr, synth_t* allSynth_ptr) {
  static boolean lastState[MAX_BLOBS] = {false};
  
  AudioNoInterrupts();
  for (blob_t* blob_ptr = (blob_t *)ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = (blob_t *)ITERATOR_NEXT(blob_ptr)) {

    if (blob_ptr->UID < MAX_SYNTH) {

      if (blob_ptr->state && !blob_ptr->lastState) {
        allSynth_ptr[blob_ptr->UID].wf_ptr->phase(0);
        allSynth_ptr[blob_ptr->UID].fade_ptr->fadeIn(3);
      }

      if (blob_ptr->state) {
        allSynth_ptr[blob_ptr->UID].wf_ptr->frequency(blob_ptr->centroid.X * 3);
        allSynth_ptr[blob_ptr->UID].fm_ptr->frequency(blob_ptr->centroid.Y * 4 + 50);
        /*
          if (blob_ptr->UID == 0) {
          allSynth_ptr[blob_ptr->UID].wf_ptr->frequency((blob_ptr->centroid.X / 4.0) + 1);
          allSynth_ptr[blob_ptr->UID].fm_ptr->frequency((blob_ptr->centroid.Y / 2.0) + 5);
          }
          else if (blob_ptr->UID == 1) {
          allSynth_ptr[blob_ptr->UID].wf_ptr->frequency(blob_ptr->centroid.X * 3);
          allSynth_ptr[blob_ptr->UID].fm_ptr->frequency(blob_ptr->centroid.Y * 4 + 50);
          }
        */
      }
      else {
        allSynth_ptr[blob_ptr->UID].fade_ptr->fadeOut(500);
      }
    }
  }
  AudioInterrupts();
}
