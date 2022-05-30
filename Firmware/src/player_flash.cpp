/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "player_flash.h"

#include "config.h"
#include "llist.h"
#include "blob.h"
#include "midi_bus.h"
#include "mapping_lib.h"

#include <Audio.h>         // https://github.com/PaulStoffregen/Audio
#include <Wire.h>          // https://github.com/PaulStoffregen/Wire
#include <SPI.h>           // https://github.com/PaulStoffregen/SPI
#include <SD.h>            // https://github.com/PaulStoffregen/SD
#include <SerialFlash.h>   // https://github.com/PaulStoffregen/SerialFlash

#if defined(PLAYER_FLASH)
#define MEM_MOSI   11  // Teensy4.0 hardware SPI
#define MEM_MISO   12  // Teensy4.0 hardware SPI
#define MEM_SCK    13  // Teensy4.0 hardware SPI
#define MEM_SC     6   // Teensy4.0 hardware SPI

AudioOutputI2S            i2s_OUT;
AudioPlaySerialflashRaw   playFlashRaw;
AudioConnection           patchCord1(playFlashRaw, 0, i2s_OUT, 0);
AudioConnection           patchCord2(playFlashRaw, 0, i2s_OUT, 1);

void player_flash_setup(void) {
  while (!SerialFlash.begin(6));
}

void player_flash(void) {
  blob_t* blob_ptr = (blob_t*)llist_blobs.tail_ptr;
  if (blob_ptr != NULL) {
    //AudioNoInterrupts();
    if (blob_ptr->state) {
      if (!blob_ptr->lastState) {
        playFlashRaw.play("A.RAW");
      }
      else {
        // NOTHINK YET
      }
      //AudioInterrupts();
    }
    else {
      // NOTHINK YET
    };
  };
};
#endif
