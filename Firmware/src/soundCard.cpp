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

// According to https://github.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
void update_levels(void) {

  switch (currentLevel) {
    case LINE_OUT:
      // FONCTION : line_out level adjustment using rotary encoder // DEFAULT MODE
      // LOWEST level is 31 (1.16 Volts p-p)
      // HIGHEST level is 13 (3.16 Volts p-p)
      if (levels[LINE_OUT].run) {
        levels[LINE_OUT].run = false;
        AudioNoInterrupts();
        //sgtl5000.dacVolume(levels_ptr[LINE_OUT].val);
        //sgtl5000.volume(levels[LINE_OUT].val); // DO NOT WORK!?
        AudioInterrupts();
      }
      break;
    case SIG_IN:
      // FONCTION : sig-in level adjustment using rotary encoder
      // LOWEST level is 15 (0.24 Volts p-p)
      // HIGHEST level is 0 (3.12 Volts p-p)
      if (levels[SIG_IN].run) {
        levels[SIG_IN].run = false;
        AudioNoInterrupts();
        sgtl5000.lineInLevel(levels[SIG_IN].val);
        //sgtl5000.micGain(levels[SIG_IN].val);
        AudioInterrupts();
      }
      break;
    case SIG_OUT:
      // FONCTION : sig_out level adjustment using rotary encoder
      // LOWEST level is 31
      // HIGHEST level is 13
      if (levels[SIG_OUT].run) {
        levels[SIG_OUT].run = false;
        AudioNoInterrupts();
        sgtl5000.lineOutLevel(levels[SIG_OUT].val);
        AudioInterrupts();
      };
      break;
  };
};