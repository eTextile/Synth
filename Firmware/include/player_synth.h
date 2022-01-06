/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PLAYER_SYNTH_H__
#define __PLAYER_SYNTH_H__

#include <Audio.h>         // https://github.com/PaulStoffregen/Audio

typedef struct synth synth_t;
struct synth {
  AudioSynthWaveform* wf;
  AudioSynthWaveformSineModulated* fm;
  AudioEffectFade* fade;
  AudioMixer4* mix;
};

void PLAYER_SYNTH_SETUP(void);
void player_synth(void);

#endif /*__PLAYER_SYNTH_H__*/