/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "soundCard.h"

void SETUP_SOUND_CARD(AudioControlSGTL5000* soundCard_ptr) {
  AudioMemory(100);
  soundCard_ptr->enable();
  //soundCard_ptr->inputSelect(AUDIO_INPUT_LINEIN);
  soundCard_ptr->inputSelect(AUDIO_INPUT_MIC);
}

void update_volumes(
  presetMode_t curentMode,
  preset_t* presets_ptr,
  AudioControlSGTL5000* soundCard_ptr
) {

  switch (curentMode) {

    case LINE_OUT:
      // FONCTION : line_out level adjustment using rotary encoder // DEFAULT MODE
      // According to https://github.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
      // LOWEST level is 31 (1.16 Volts p-p)
      // HIGHEST level is 13 (3.16 Volts p-p)
      if (presets_ptr[LINE_OUT].update) {
        presets_ptr[LINE_OUT].update = false;
        AudioNoInterrupts();
        soundCard_ptr->dacVolume(presets_ptr[LINE_OUT].val);
        AudioInterrupts();
#if DEBUG_ENCODER
        Serial.printf("\nLINE_OUT: %d", presets_ptr[LINE_OUT].val);
#endif
      }
      break;

    case SIG_IN:
      // FONCTION : sig-in level adjustment using rotary encoder
      // According to https://githpresetub.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
      // LOWEST level is 15 (0.24 Volts p-p)
      // HIGHEST level is 0 (3.12 Volts p-p)
      if (presets_ptr[SIG_IN].update) {
        presets_ptr[SIG_IN].update = false;
        AudioNoInterrupts();
        //soundCard_ptr->lineInLevel(presets_ptr[SIG_IN].val);
        soundCard_ptr->micGain(presets_ptr[SIG_IN].val);
        AudioInterrupts();
#if DEBUG_ENCODER
        Serial.printf("\nSIG_IN : %d", presets_ptr[SIG_IN].val);
#endif
      }
      break;

    case SIG_OUT:
      // FONCTION : sig_out level adjustment using rotary encoder
      // According to https://github.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
      // LOWEST level is 31
      // HIGHEST level is 13
      if (presets_ptr[SIG_OUT].update) {
        presets_ptr[SIG_OUT].update = false;
        AudioNoInterrupts();
        soundCard_ptr->lineOutLevel(presets_ptr[SIG_OUT].val);
        AudioInterrupts();
#if DEBUG_ENCODER
        Serial.printf("\nSIG_OUT : %d", presets_ptr[SIG_OUT].val);
#endif
      }
      break;
  }
}
