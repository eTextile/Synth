/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "soundCard.h"

void SETUP_SOUND_CARD(AudioControlSGTL5000* soundCard_ptr, preset_t* presets_ptr) {
  AudioNoInterrupts();
  AudioMemory(100);
  soundCard_ptr->enable();
  soundCard_ptr->inputSelect(AUDIO_INPUT_LINEIN);
  soundCard_ptr->dacVolume(presets_ptr[LINE_OUT].val);
  soundCard_ptr->lineInLevel(presets_ptr[SIG_IN].val);
  soundCard_ptr->lineOutLevel(presets_ptr[SIG_OUT].val);
  AudioInterrupts();
}

void set_volumes(AudioControlSGTL5000* soundCard_ptr, preset_t* presets_ptr) {

  AudioNoInterrupts();
  // FONCTION : line_out level adjustment using rotary encoder // DEFAULT MODE
  // According to https://github.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
  // LOWEST level is 31 (1.16 Volts p-p)
  // HIGHEST level is 13 (3.16 Volts p-p)
  if (presets_ptr[LINE_OUT].update) {
    presets_ptr[LINE_OUT].update = false;
    uint8_t val = abs((presets_ptr[LINE_OUT].val - presets_ptr[LINE_OUT].minVal) - (presets_ptr[LINE_OUT].maxVal - presets_ptr[LINE_OUT].minVal)) + presets_ptr[LINE_OUT].minVal;
    soundCard_ptr->dacVolume(val);
#if DEBUG_ENCODER
    Serial.printf("\nLINE_OUT: %d", val);
#endif
  }

  // FONCTION : sig-in level adjustment using rotary encoder
  // According to https://githpresetub.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
  // LOWEST level is 15 (0.24 Volts p-p)
  // HIGHEST level is 0 (3.12 Volts p-p)
  if (presets_ptr[SIG_IN].update) {
    presets_ptr[SIG_IN].update = false;
    uint8_t val = abs((presets_ptr[SIG_IN].val - presets_ptr[SIG_IN].minVal) - (presets_ptr[SIG_IN].maxVal - presets_ptr[SIG_IN].minVal)) + presets_ptr[SIG_IN].minVal;
    soundCard_ptr->lineInLevel(val);
#if DEBUG_ENCODER
    Serial.printf("\nSIG_IN : %d", val);
#endif
  }

  // FONCTION : sig_out level adjustment using rotary encoder
  // According to https://github.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
  // LOWEST level is 31
  // HIGHEST level is 13
  if (presets_ptr[SIG_OUT].update) {
    presets_ptr[SIG_OUT].update = false;
    uint8_t val = abs((presets_ptr[SIG_OUT].val - presets_ptr[SIG_OUT].minVal) - (presets_ptr[SIG_OUT].maxVal - presets_ptr[SIG_OUT].minVal)) + presets_ptr[SIG_OUT].minVal;
    soundCard_ptr->lineOutLevel(val);
#if DEBUG_ENCODER
    Serial.printf("\nSIG_OUT : %d", val);
#endif
  }
  AudioInterrupts();
}
