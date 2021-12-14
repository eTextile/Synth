/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"
#include "midi_bus.h"
#include "usb_midi_transmit.h"

#if defined(HARDWARE_MIDI)
#include "hardware_midi_transmit.h"
#endif
#if defined(MAPPING_LAYOUT)
#include "mapping_lib.h"
#endif
#if defined(PLAYER_FLASH)
#include "player_flash.h"
#endif
#if defined(PLAYER_SYNTH)
#include "player_synth.h"
#endif
#if defined(PLAYER_GRANULAR)
#include "player_granular.h"
#endif
#if defined(PLAYER_FLASH) || defined(PLAYER_SYNTH) || defined(PLAYER_SYNTH2) || defined(PLAYER_GRANULAR)
#include "soundCard.h"
#endif

unsigned long fpsTimeStamp = 0;
uint16_t fps = 0;

void setup() {
  SCAN_SETUP();
  INTERP_SETUP();
  BLOB_SETUP();
  MIDI_SETUP();
  USB_MIDI_TRANSMIT_SETUP();
  CONFIG_SETUP();

#if defined(HARDWARE_MIDI)
  HARDWARE_MIDI_TRANSMIT_SETUP();
#endif
#if defined(RUNING_MEDIAN)
  RUNING_MEDIAN_SETUP();
#endif
#if defined(MAPPING_LAYOUT)
  MAPPING_LIB_SETUP();
#endif
#if defined(SOUND_CARD)
  SOUND_CARD_SETUP();
#endif
#if defined(PLAYER_SYNTH)
  //PLAYER_SYNTH_SETUP();
  PLAYER_SYNTH2_SETUP();
#endif
#if defined(PLAYER_SYNTH2)
  PLAYER_SYNTH2_SETUP();
#endif
#if defined(PLAYER_FLASH)
  PLAYER_FLASH_SETUP();
#endif
#if defined(PLAYER_GRANULAR)
  PLAYER_GRANULAR_SETUP();
#endif
};

void loop() {
  usb_midi_read_input();
  update_config();
#if defined(HARDWARE_MIDI)
  hardware_midi_read_input();
#endif
#if defined(SOUND_CARD)
  update_levels();
#endif
  matrix_calibrate();
  matrix_scan();
  matrix_interp();
  matrix_find_blobs();
#if defined(MAPPING_LAYOUT)
  mapping_lib_update();
#endif
#if defined(PLAYER_SYNTH)
  player_synth();
#endif
#if defined(PLAYER_SYNTH2)
  player_synth2();
#endif
#if defined(PLAYER_GRANULAR)
  player_granular();
#endif
#if defined(PLAYER_FLASH)
  player_flash();
#endif
  usb_midi_transmit();
#if defined(HARDWARE_MIDI)
  hardware_midi_transmit();
#endif
#if defined(DEBUG_FPS)
  if (millis() - fpsTimeStamp >= 1000) {
    fpsTimeStamp = millis();
    Serial.printf("\nFPS:%d", fps);
    //Serial.printf("\nFPS:%d\tCPU:%f\tMEM:%f", fps, AudioProcessorUsageMax(), AudioMemoryUsageMax());
    //AudioProcessorUsageMaxReset();
    //AudioMemoryUsageMaxReset();
    fps = 0;
  };
  fps++;
#endif
};