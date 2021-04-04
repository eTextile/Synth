/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PLAYER_SYNTH_H__
#define __PLAYER_SYNTH_H__

#include <Audio.h>              // https://github.com/PaulStoffregen/Audio
#include "config.h"
#include "llist.h"
#include "blob.h"
#include "mapping.h"

typedef struct llist llist_t;       // Forward declaration
typedef struct blob blob_t;         // Forward declaration

typedef struct synth synth_t;
struct synth {
  AudioSynthWaveform* wf_ptr;
  AudioSynthWaveformSineModulated* fm_ptr;
  AudioEffectFade* fade_ptr;
  AudioMixer4* mix_ptr;
};

void SYNTH_PLAYER_SETUP(synth_t* allSynth_ptr);
void synth_player(llist_t* blobs_ptr, synth_t* allSynth_ptr);

#endif /*__PLAYER_SYNTH_H__*/
