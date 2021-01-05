/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "audio.h"

void SETUP_DAC(
  preset_t* presets_ptr,
  AudioControlSGTL5000* dac_ptr,
  AudioSynthWaveform* wfA_ptr,
  AudioSynthWaveformSineModulated* sine_fm_ptr,
  AudioEffectFade* fade_ptr
) {

  AudioMemory(20);

  dac_ptr->enable();

  dac_ptr->dacVolume(presets_ptr[LINE_OUT].val);
  dac_ptr->lineInLevel(presets_ptr[SIG_IN].val);
  dac_ptr->lineOutLevel(presets_ptr[SIG_OUT].val);
  fade_ptr->fadeOut(0);
  wfA_ptr->amplitude(1.0);
  sine_fm_ptr->amplitude(1.0);

  wfA_ptr->begin(WAVEFORM_SINE);
}

void make_noise(
  preset_t* presets_ptr,
  llist_t* blobs_ptr,
  AudioControlSGTL5000* dac_ptr,
  AudioSynthWaveform* wfA_ptr,
  AudioSynthWaveformSineModulated* sine_fm_ptr,
  AudioEffectFade* fade_ptr
) {
  static uint8_t lastAlive = 0;

  if (presets_ptr[LINE_OUT].val != presets_ptr[LINE_OUT].lastVal) {
    dac_ptr->dacVolume(presets_ptr[LINE_OUT].val);
  }

  for (blob_t* blob = ITERATOR_START_FROM_HEAD(blobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {

    AudioNoInterrupts();
    if (blob->UID == 0) {
      if (blob->alive == 1 && lastAlive == 0) {
        fade_ptr->fadeIn(80);
      }
      if (blob->alive == 1) {
        wfA_ptr->frequency((blob->centroid.X / 4) + 10);
        //wfA_ptr->phase((blob->box.D / 64) * 360.0);
        sine_fm_ptr->frequency((blob->centroid.Y / 8.0) + 8);
      }
      else {
        fade_ptr->fadeOut(500);
        //wfA_ptr->frequency(0);
        //sine_fm_ptr->frequency(0);
      }
      lastAlive = blob->alive;
    }
    AudioInterrupts();
    /*
      blob->UID;        // uint8_t unique session ID
      blob->alive;      // uint8_t
      blob->centroid.X; // uint8_t
      blob->centroid.Y; // uint8_t
      blob->box.W;      // uint8_t
      blob->box.H;      // uint8_t
      blob->box.D;      // uint8_t
    */
  }
}
