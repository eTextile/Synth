/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "soundCard.h"

AudioControlSGTL5000  sgtl5000;

void SOUND_CARD_SETUP() {
  AudioMemory(100);
  sgtl5000.enable();
  sgtl5000.inputSelect(AUDIO_INPUT_LINEIN);
  //sgtl5000.inputSelect(AUDIO_INPUT_MIC);
}

void update_levels(void) {

  switch (currentMode) {
    case LINE_OUT:
      // FONCTION : line_out level adjustment using rotary encoder // DEFAULT MODE
      // According to https://github.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
      // LOWEST level is 31 (1.16 Volts p-p)
      // HIGHEST level is 13 (3.16 Volts p-p)
      if (presets[LINE_OUT].update) {
        presets[LINE_OUT].update = false;
        AudioNoInterrupts();
        //sgtl5000.dacVolume(presets_ptr[LINE_OUT].val);
        //sgtl5000.volume(presets[LINE_OUT].val); // DO NOT WORK!?
        AudioInterrupts();
        presets[LINE_OUT].setLed = true;
        presets[LINE_OUT].updateLed = true;
      }
      break;
    case SIG_IN:
      // FONCTION : sig-in level adjustment using rotary encoder
      // According to https://githpresetub.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
      // LOWEST level is 15 (0.24 Volts p-p)
      // HIGHEST level is 0 (3.12 Volts p-p)
      if (presets[SIG_IN].update) {
        presets[SIG_IN].update = false;
        AudioNoInterrupts();
        sgtl5000.lineInLevel(presets[SIG_IN].val);
        //sgtl5000.micGain(presets[SIG_IN].val);
        AudioInterrupts();
        presets[SIG_IN].setLed = true;
        presets[SIG_IN].updateLed = true;
      }
      break;
    case SIG_OUT:
      // FONCTION : sig_out level adjustment using rotary encoder
      // According to https://github.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
      // LOWEST level is 31
      // HIGHEST level is 13
      if (presets[SIG_OUT].update) {
        presets[SIG_OUT].update = false;
        AudioNoInterrupts();
        sgtl5000.lineOutLevel(presets[SIG_OUT].val);
        AudioInterrupts();
        presets[SIG_OUT].setLed = true;
        presets[SIG_OUT].updateLed = true;
      };
      break;
  };
};
