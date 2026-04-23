/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "scan.h"
#include "interp.h"
#include "llist.h"
#include "blob.h"
#include "median.h"
#include "mapping.h"
#include "usb_midi_io.h"
#include "hardware_midi_io.h"
#include "sound_card.h"

uint32_t fpsTimeStamp = 0;
uint16_t fps = 0;

void setup() {
  #if defined(USB_MIDI_SERIAL)
    //while (!Serial);
    //Serial.printf("\nVERSION:\t%s", VERSION);
  #endif
  hardware_setup();
  scan_setup();
  interp_setup();
  llist_setup();
  blob_setup();
  hardware_midi_setup();
  usb_midi_setup();
  midi_bus_setup();
  matrix_calibrate();
  
  if (load_flash_config()) {
    if (mappings_apply_config(flash_config_ptr, flash_config_size)) {
      //
    }
    else {
      set_mode(ERROR_MODE);
    }
  }
  set_mode(PENDING_MODE);
  bootTime = millis();
};

void loop() {

  update_controls();
  update_levels();

  switch (e256_current_mode) {

    case PENDING_MODE:
      usb_midi_recive();
      if ((millis() - bootTime) > PENDING_MODE_TIMEOUT) {
        set_mode(STANDALONE_MODE);
      }
      break;

    case SYNC_MODE:
      usb_midi_recive();
      break;

    case MATRIX_RAW_MODE:
      usb_midi_recive();
      matrix_scan();
      usb_midi_transmit_raw_matrix();
      break;
    /*
    case MATRIX_INTERP_MODE: // FIXME
      usb_midi_recive();
      matrix_scan();
      usb_midi_transmit_interp_matrix();
      break;
    */
    case EDIT_MODE:
      usb_midi_recive();
      matrix_scan();
      matrix_interp();
      matrix_find_blobs();
      usb_midi_transmit_blobs();
      break;

    case THROUGH_MODE:
      usb_midi_recive();
      hardware_midi_transmit_mappings_midi_msg();
      break;
      
    case PLAY_MODE:
      usb_midi_recive();
      matrix_scan();
      matrix_interp();
      matrix_find_blobs();
      mapping_lib_update();
      usb_midi_transmit_mappings_midi_msg();
      hardware_midi_transmit_mappings_midi_msg();
      break;

    case STANDALONE_MODE:
      hardware_midi_recive(); // TESTING!
      matrix_scan();
      matrix_interp();
      matrix_find_blobs();
      mapping_lib_update();
      hardware_midi_transmit_mappings_midi_msg();
      break;

    default:
      usb_midi_recive();
      break;
  };
  
  midi_msg_t* midi_msg_ptr = NULL;
  while ((midi_msg_ptr = (midi_msg_t*)llist_pop_front(&llist_midi_out)) != NULL); // Save/rescure all midi_out llist nodes
  
  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FPS)
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
