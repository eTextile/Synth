/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PLAYER_GRANULAR_H__
#define __PLAYER_GRANULAR_H__

#include <Audio.h>              // https://github.com/PaulStoffregen/Audio
#include "config.h"
#include "llist.h"
#include "blob.h"
#include "mapping.h"

typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

void SETUP_GRANULAR(AudioEffectGranular* granular_ptr, uint16_t* buffer_ptr);
void granular_player(llist_t* blobs_ptr, AudioEffectGranular* granular_ptr, uint16_t* buffer_ptr);

#endif /*__PLAYER_GRANULAR_H__*/
