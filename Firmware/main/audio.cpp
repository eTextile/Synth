/*
  This file is part of the eTextile-matrix-sensor project - http://matrix.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

//https://www.pjrc.com/teensy/td_libs_Audio.html
#include "audio.h"

void SETUP_DAC(
  AudioControlSGTL5000* dac_ptr,
  preset_t* presets_ptr,
  AudioSynthWaveform* wfA_ptr,
  AudioSynthWaveform* wfB_ptr
) {

  AudioMemory(10);

  dac_ptr->enable();
  dac_ptr->dacVolume(presets_ptr[LINE_OUT].val);
  //dac_ptr->lineInLevel(presets_ptr[SIG_IN].val);
  dac_ptr->lineOutLevel(presets_ptr[SIG_OUT].val);

  //wfA_ptr->frequency(440);
  //wfB_ptr->frequency(440);

  wfA_ptr->amplitude(1.0);
  wfB_ptr->amplitude(1.0);

  wfA_ptr->begin(WAVEFORM_SINE);
  wfB_ptr->begin(WAVEFORM_SINE);
}

void e256_make_noise(
  llist_t* blobs_ptr,
  AudioControlSGTL5000* dac_ptr,
  AudioSynthWaveform* wfA_ptr,
  AudioSynthWaveform* wfB_ptr
) {

  for (blob_t* blob = ITERATOR_START_FROM_HEAD(blobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {

    AudioNoInterrupts();
    if (blob->alive == 1) {
      wfA_ptr->frequency(blob->centroid.X * 10.0);
      wfB_ptr->frequency(blob->centroid.Y * 10.0);
      //waveform1.phase(knob_A3 * 360.0);
    }
    else {
      wfA_ptr->frequency(0);
      wfB_ptr->frequency(0);
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
