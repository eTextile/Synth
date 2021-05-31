/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PLAYER_GRANULAR_H__
#define __PLAYER_GRANULAR_H__

#include <Audio.h>         // https://github.com/PaulStoffregen/Audio
#include <Wire.h>          // https://github.com/PaulStoffregen/Wire
#include <SPI.h>           // https://github.com/PaulStoffregen/SPI
#include <SD.h>            // https://github.com/PaulStoffregen/SD
#include <SerialFlash.h>   // https://github.com/PaulStoffregen/SerialFlash

#include "config.h"
#include "llist.h"
#include "blob.h"

typedef struct lnode lnode_t;   // Forward declaration
typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

void GRANULAR_PLAYER_SETUP(void);
void granular_player(llist_t* llist_ptr);

#endif /*__PLAYER_GRANULAR_H__*/
