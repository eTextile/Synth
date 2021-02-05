/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "audio.h"

void SETUP_DAC(
  AudioControlSGTL5000* dac_ptr,
  preset_t* presets_ptr,
  synth_t* allSynth_ptr
) {

  AudioMemory(20);
  dac_ptr->enable();
  dac_ptr->inputSelect(AUDIO_INPUT_LINEIN);

  dac_ptr->dacVolume(presets_ptr[LINE_OUT].val);
  dac_ptr->lineInLevel(presets_ptr[SIG_IN].val);
  dac_ptr->lineOutLevel(presets_ptr[SIG_OUT].val);

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

  while (!SerialFlash.begin(6));
}


void set_volumes(
  AudioControlSGTL5000* dac_ptr,
  preset_t* presets_ptr
) {

  if (presets_ptr[LINE_OUT].val != presets_ptr[LINE_OUT].lastVal) {
    //dac_ptr->dacVolume(presets_ptr[LINE_OUT].val); // FIXME!
    dac_ptr->volume(presets_ptr[LINE_OUT].val);
  }
  if (presets_ptr[SIG_OUT].val != presets_ptr[SIG_OUT].lastVal) {
    dac_ptr->lineOutLevel(presets_ptr[SIG_OUT].val);
  }
  if (presets_ptr[SIG_IN].val != presets_ptr[SIG_IN].lastVal) {
    dac_ptr->lineInLevel(presets_ptr[SIG_IN].val);
  }
}

/////////////////////////// MAKE NOISE FONCTION !
void make_noise(
  AudioControlSGTL5000* dac_ptr,
  llist_t* blobs_ptr,
  synth_t* allSynth_ptr
) {

  AudioNoInterrupts();

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {

    if (blob_ptr->UID < MAX_SYNTH) {

      if (blob_ptr->alive && allSynth_ptr[blob_ptr->UID].lastBlobState == 0) {
        allSynth_ptr[blob_ptr->UID].wf_ptr->phase(0);
        allSynth_ptr[blob_ptr->UID].fade_ptr->fadeIn(3);
      }

      if (blob_ptr->alive) {
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
      allSynth_ptr[blob_ptr->UID].lastBlobState = blob_ptr->alive;
    }
  }
  AudioInterrupts();
}
