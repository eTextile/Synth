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

  AudioMemory(30);

  dac_ptr->enable();
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

  allSynth_ptr[5].mix_ptr->gain(0, 0.25);
  allSynth_ptr[5].mix_ptr->gain(1, 0.25);
  allSynth_ptr[5].mix_ptr->gain(2, 0.25);
  allSynth_ptr[5].mix_ptr->gain(3, 0.25);
}


void set_volumes(
  AudioControlSGTL5000* dac_ptr,
  preset_t* presets_ptr
) {

  if (presets_ptr[LINE_OUT].val != presets_ptr[LINE_OUT].lastVal) {
    dac_ptr->dacVolume(presets_ptr[LINE_OUT].val);
  }
  if (presets_ptr[SIG_OUT].val != presets_ptr[SIG_OUT].lastVal) {
    dac_ptr->lineOutLevel(presets_ptr[LINE_OUT].val);
  }
  if (presets_ptr[SIG_IN].val != presets_ptr[SIG_IN].lastVal) {
    dac_ptr->lineInLevel(presets_ptr[LINE_OUT].val);
  }
}

/////////////////////////// MAKE NOISE FONCTION !
void make_noise(
  AudioControlSGTL5000* dac_ptr,
  llist_t* blobs_ptr,
  synth_t* allSynth_ptr
) {

  AudioNoInterrupts();

  for (blob_t* blob = ITERATOR_START_FROM_HEAD(blobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {

    if (blob->UID < MAX_SYNTH) {

      if (blob->alive && allSynth_ptr[blob->UID].lastBlobState == 0) {
        allSynth_ptr[blob->UID].wf_ptr->phase(0);
        allSynth_ptr[blob->UID].fade_ptr->fadeIn(3);
      }
      if (blob->alive) {
        allSynth_ptr[blob->UID].wf_ptr->frequency((blob->centroid.X / 4.0) + 1);
        allSynth_ptr[blob->UID].fm_ptr->frequency((blob->centroid.Y / 2.0) + 5);
        //allSynth_ptr[blob->UID].wf_ptr->frequency(blob->centroid.X * 3);
        //allSynth_ptr[blob->UID].fm_ptr->frequency(blob->centroid.Y * 4 + 50);
      }
      else {
        allSynth_ptr[blob->UID].fade_ptr->fadeOut(500);
      }
      allSynth_ptr[blob->UID].lastBlobState = blob->alive;
    }
  }
  AudioInterrupts();
}

// TODO
void tapTempo(tSwitch_t* tSwitch_ptr, cSlider_t* slider_ptr) {

}

// [UID, alive, CX, CY, W, H, D]
