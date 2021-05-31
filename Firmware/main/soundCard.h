/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __SOUND_CARD_H__
#define __SOUND_CARD_H__

#include "config.h"
#include "presets.h"

#include <Audio.h>              // https://github.com/PaulStoffregen/Audio

typedef struct preset preset_t; // Forward declaration

extern AudioControlSGTL5000  sgtl5000;

void SOUND_CARD_SETUP();

void update_volumes(
  presetMode_t curentMode,
  preset_t* presets_ptr
);

#endif /*__SOUND_CARD_H__*/
