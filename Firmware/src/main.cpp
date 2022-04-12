/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"
#include "median.h"
#include "midi_bus.h"
#include "mapping_lib.h"
#include "usb_midi_transmit.h"
#include "hardware_midi_transmit.h"
#include "soundCard.h"

#if defined(PLAYER_FLASH)
#include "player_flash.h"
#endif
#if defined(PLAYER_SYNTH)
#include "player_synth.h"
#endif
#if defined(PLAYER_GRANULAR)
#include "player_granular.h"
#endif

unsigned long fpsTimeStamp = 0;
uint16_t fps = 0;

void setup() {
  hardware_setup();
  scan_setup();
  interp_setup();
  blob_setup();
  midi_bus_setup();
  // config_setup();
  // mapping_lib_setup();
  usb_midi_transmit_setup();
  hardware_midi_transmit_setup();
  set_mode(e256_mode);
  sound_card_setup();
#if defined(RUNING_MEDIAN)
  running_median_setup();
#endif
#if defined(PLAYER_SYNTH)
  player_synth_setup();
#endif
#if defined(PLAYER_SYNTH2)
  player_synth2_setup();
#endif
#if defined(PLAYER_FLASH)
  player_flash_setup();
#endif
#if defined(PLAYER_GRANULAR)
  player_granular_setup();
#endif
};

void loop() {
  matrix_scan();
  matrix_interp();
  matrix_find_blobs();
  update_controls();

  switch (e256_mode) {
  case PENDING_MODE:
    usb_midi_read_input();
    usb_midi_transmit();
    usb_midi_pending_mode_timeout();
    break;
  case SYNC_MODE:
    usb_midi_read_input();
    usb_midi_transmit();
    break;
  case STANDALONE_MODE:
    hardware_midi_read_input();
    // mapping_lib_update();
    update_levels();
    #if defined(PLAYER_SYNTH)
      player_synth();
    #endif
    #if defined(PLAYER_SYNTH2)
      player_synth2();
    #endif
    #if defined(PLAYER_FLASH)
      player_flash();
    #endif
    #if defined(PLAYER_GRANULAR)
      player_granular();
    #endif
    hardware_midi_transmit();
    break;
  case MATRIX_MODE_RAW:
    usb_midi_read_input();
    usb_midi_transmit();
    break;
  case EDIT_MODE:
    usb_midi_read_input();
    usb_midi_transmit();
    break;
  case PLAY_MODE:
    usb_midi_read_input();
    // mapping_lib_update();
    usb_midi_transmit();
    break;
  }

#if defined(DEBUG_FPS)
  if (millis() - fpsTimeStamp >= 1000) {
    fpsTimeStamp = millis();
    Serial.printf("\nFPS:%d", fps);
    // Serial.printf("\nFPS:%d\tCPU:%f\tMEM:%f", fps, AudioProcessorUsageMax(), AudioMemoryUsageMax());
    // AudioProcessorUsageMaxReset();
    // AudioMemoryUsageMaxReset();
    fps = 0;
  };
  fps++;
#endif
};