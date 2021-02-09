/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "player_raw.h"

void SETUP_FLASH_PLAYER() {
  while (!SerialFlash.begin(6));
}

void play_raw(llist_t* blobs_ptr, AudioPlaySerialflashRaw* player_ptr) {
  player_ptr->play("A.RAW");
}
