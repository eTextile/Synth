/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PLAYER_SYNTH_H__
#define __PLAYER_SYNTH_H__

#include <Audio.h>         // https://github.com/PaulStoffregen/Audio
#include <Wire.h>          // https://github.com/PaulStoffregen/Wire
#include <SPI.h>           // https://github.com/PaulStoffregen/SPI
#include <SD.h>            // https://github.com/PaulStoffregen/SD
#include <SerialFlash.h>   // https://github.com/PaulStoffregen/SerialFlash

#include "config.h"
#include "llist.h"
#include "blob.h"
#include "mapping.h"

typedef struct llist llist_t;       // Forward declaration
typedef struct blob blob_t;         // Forward declaration

extern llist_t blobs;               // Located in blob.cpp

typedef struct synth synth_t;
struct synth {
  AudioSynthWaveform* wf;
  AudioSynthWaveformSineModulated* fm;
  AudioEffectFade* fade;
  AudioMixer4* mix;
};

void SYNTH_PLAYER_SETUP(void);
void synth_player(void);

#endif /*__PLAYER_SYNTH_H__*/
