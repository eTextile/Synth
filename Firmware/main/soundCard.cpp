/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "soundCard.h"

void SETUP_SOUND_CARD(
  AudioControlSGTL5000* soundCard_ptr,
  preset_t* presets_ptr
) {
  AudioNoInterrupts();
  AudioMemory(100);
  soundCard_ptr->enable();
  soundCard_ptr->inputSelect(AUDIO_INPUT_LINEIN);
  soundCard_ptr->dacVolume(presets_ptr[LINE_OUT].val);
  soundCard_ptr->lineInLevel(presets_ptr[SIG_IN].val);
  soundCard_ptr->lineOutLevel(presets_ptr[SIG_OUT].val);
  AudioInterrupts();
}

void set_volumes(
  AudioControlSGTL5000* soundCard_ptr,
  preset_t* presets_ptr
) {
  AudioNoInterrupts();
  if (presets_ptr[LINE_OUT].val != presets_ptr[LINE_OUT].lastVal) {
    //soundCard_ptr->dacVolume(presets_ptr[LINE_OUT].val); // FIXME!
    soundCard_ptr->volume(presets_ptr[LINE_OUT].val);
  }
  if (presets_ptr[SIG_OUT].val != presets_ptr[SIG_OUT].lastVal) {
    soundCard_ptr->lineOutLevel(presets_ptr[SIG_OUT].val);
  }
  if (presets_ptr[SIG_IN].val != presets_ptr[SIG_IN].lastVal) {
    soundCard_ptr->lineInLevel(presets_ptr[SIG_IN].val);
  }
  AudioInterrupts();
}
