/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "audio.h"

void SETUP_DAC(
  preset_t* presets_ptr,
  synth_t* allSynth_ptr,
  AudioControlSGTL5000* dac_ptr
) {

  AudioMemory(20);

  dac_ptr->enable();
  dac_ptr->dacVolume(presets_ptr[LINE_OUT].val);
  dac_ptr->lineInLevel(presets_ptr[SIG_IN].val);
  dac_ptr->lineOutLevel(presets_ptr[SIG_OUT].val);

  for (int i = 0; i < 2; i++) {
    allSynth_ptr[i].fade_ptr->fadeOut(0);
    allSynth_ptr[i].wf_ptr->begin(WAVEFORM_SINE);
    allSynth_ptr[i].wf_ptr->amplitude(0.9);
    allSynth_ptr[i].fm_ptr->amplitude(0.9);
  }
  allSynth_ptr[0].mix_ptr->gain(0, 0.5);
  allSynth_ptr[0].mix_ptr->gain(1, 0.5);
}

/////////////////////////// MAKE NOISE FONCTION !
void make_noise(
  preset_t* presets_ptr,
  llist_t* blobs_ptr,
  synth_t* allSynth_ptr,
  AudioControlSGTL5000* dac_ptr
) {

  static uint8_t lastState = 0;

  if (presets_ptr[LINE_OUT].val != presets_ptr[LINE_OUT].lastVal) {
    dac_ptr->dacVolume(presets_ptr[LINE_OUT].val);
  }

  // TODO : Add 8 Synthetizers for multitouch polyphonic
  for (blob_t* blob = ITERATOR_START_FROM_HEAD(blobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {

    AudioNoInterrupts();

    if (blob->alive == 1 && allSynth_ptr[blob->UID].lastBlobState == 0) {
      allSynth_ptr[blob->UID].wf_ptr->phase(0);
      allSynth_ptr[blob->UID].fade_ptr->fadeIn(1);
    }
    if (blob->alive) {
      //allSynth_ptr[blob->UID].wf_ptr->frequency((blob->centroid.X / 4.0) + 1);
      //allSynth_ptr[blob->UID].fm_ptr->frequency((blob->centroid.Y / 2.0) + 1);
      allSynth_ptr[blob->UID].wf_ptr->frequency(blob->centroid.X * 2);
      allSynth_ptr[blob->UID].fm_ptr->frequency(blob->centroid.Y * 2);
    }
    else {
      allSynth_ptr[blob->UID].fade_ptr->fadeOut(500);
    }
    allSynth_ptr[blob->UID].lastBlobState = blob->alive;
  }
  AudioInterrupts();
}

/*
  blob->UID;        // uint8_t unique session ID
  blob->alive;      // uint8_t
  blob->centroid.X; // float_t
  blob->centroid.Y; // float_t
  blob->box.W;      // uint8_t
  blob->box.H;      // uint8_t
  blob->box.D;      // uint8_t
*/
