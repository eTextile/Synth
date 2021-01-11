/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"

#include <Audio.h>              // https://github.com/PaulStoffregen/Audio
#include <Wire.h>               // https://github.com/PaulStoffregen/Wire
#include <SPI.h>                // https://github.com/PaulStoffregen/SPI
#include <SD.h>                 // https://github.com/PaulStoffregen/SD
#include <SerialFlash.h>        // https://github.com/PaulStoffregen/SerialFlash

typedef struct preset preset_t; // Forward declaration
typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

void SETUP_DAC(
  preset_t* presets_ptr,
  AudioControlSGTL5000* dac_ptr,
  AudioSynthWaveform* wfA_ptr,
  AudioSynthWaveformSineModulated* sine_fm_ptr,
  AudioEffectFade* fade_ptr
);

void make_noise(
  preset_t* presets_ptr,
  llist_t* blobs_ptr,
  AudioControlSGTL5000* dac_ptr,
  AudioSynthWaveform* wfA_ptr,
  AudioSynthWaveformSineModulated* sine_fm_ptr,
  AudioEffectFade* fade_ptr
);

#endif /*__AUDIO_H__*/
