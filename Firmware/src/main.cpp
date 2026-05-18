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

void setup(void) {
  #if defined(USB_MIDI_SERIAL)
  Serial.begin(BAUD_RATE);
  while (!Serial);
    Serial.printf("\nVERSION:\t%s", VERSION);
  #endif
  hardware_setup();
  scan_setup();
  interp_setup();
  llist_setup();
  blob_setup();
  midi_bus_setup();
  hardware_midi_setup();
  usb_midi_setup();
  matrix_calibrate();
  
  if (load_flash_config()) {
    //usb_midi_send_info((uint8_t)CONFIG_LOAD_DONE, MIDI_VERBOSITY_CHANNEL); // TODO
    if (mappings_apply_config(flash_config_ptr, flash_config_size)) {
      usb_midi_send_sysex_ack((uint8_t)CONFIG_APPLY_DONE);
    }
    else {
      usb_midi_send_sysex_err((uint8_t)CONFIG_APPLY_FAILED);
      set_mode(ERROR_MODE);
    }
  }
  else {
    usb_midi_send_sysex_err((uint8_t)CONFIG_FILE_MISSING);
  }
  set_mode(PENDING_MODE);
  boot_time = millis();
};

// Per-mode function dispatch table
// ┌─────────────────┬──────────────┬─────────────┬──────────────┬────────────────┬──────────────────────────┬──────────────────────────┬───────────────────────┐
// │ Mode            │ usb_receive  │ hw_receive  │ matrix_scan  │ mapping_update │ usb_transmit             │ hw_transmit              │ note                  │
// ├─────────────────┼──────────────┼─────────────┼──────────────┼────────────────┼──────────────────────────┼──────────────────────────┼───────────────────────┤
// │ PENDING         │ ✓            │             │              │                │                          │                          │ waits PENDING_TIMEOUT │
// │ SYNC            │ ✓            │             │              │                │                          │                          │                       │
// │ MATRIX_RAW      │ ✓            │             │ ✓            │                │ transmit_raw_matrix      │                          │                       │
// │ MATRIX_INTERP   │ ✓            │             │ ✓ + interp   │                │ transmit_interp_matrix   │                          │                       │
// │ EDIT            │ ✓            │             │ ✓ + interp   │                │ transmit_blobs           │                          │                       │
// │ THROUGH         │ ✓            │             │              │                │                          │ transmit_mappings (pool) │ pool nodes returned   │
// │ PLAY            │ ✓            │             │ ✓ + interp   │ ✓              │ transmit_mappings + blobs│ transmit_mappings        │                       │
// │ STANDALONE      │              │ ✓           │ ✓ + interp   │ ✓              │                          │ transmit_mappings        │ no USB host needed    │
// └─────────────────┴──────────────┴─────────────┴──────────────┴────────────────┴──────────────────────────┴──────────────────────────┴───────────────────────┘
void loop(void) {

  update_controls();
  update_levels();

  midi_msg_t* midi_msg_ptr = NULL;

  switch (e256_current_mode) {

    case PENDING_MODE:
      usb_midi_receive();
      if ((millis() - boot_time) > PENDING_MODE_TIMEOUT) {
        set_mode(STANDALONE_MODE);
      }
      break;

    case SYNC_MODE:
      usb_midi_receive();
      break;

    case MATRIX_RAW_MODE:
      usb_midi_receive();
      matrix_scan();
      usb_midi_transmit_raw_matrix();
      break;
      
    case MATRIX_INTERP_MODE:
      usb_midi_receive();
      matrix_scan();
      matrix_interp();
      usb_midi_transmit_interp_matrix();
      break;

    case EDIT_MODE:
      usb_midi_receive();
      matrix_scan();
      matrix_interp();
      matrix_find_blobs();
      usb_midi_transmit_blobs();
      break;

    case THROUGH_MODE:
      usb_midi_receive();
      mapping_hardware_midi_transmit();
      // All nodes in llist_midi_out are pool-owned in THROUGH_MODE — return them.
      while ((midi_msg_ptr = (midi_msg_t*)llist_pop_front(&llist_midi_out)) != NULL)
        llist_push_front(&llist_midi_nodes_pool, midi_msg_ptr);
      break;
      
    case PLAY_MODE:
      usb_midi_receive();
      hardware_midi_receive();
      matrix_scan();
      matrix_interp();
      matrix_find_blobs();
      mapping_lib_update();
      mapping_hardware_midi_transmit();
      mapping_usb_midi_transmit();
      break;

    case STANDALONE_MODE:
      hardware_midi_receive();
      matrix_scan();
      matrix_interp();
      matrix_find_blobs();
      mapping_lib_update();
      mapping_hardware_midi_transmit();
      break;

    default:
      usb_midi_receive();
      break;
  };
  
  // Drain any leftover static-pointer entries from llist_midi_out (PLAY/STANDALONE modes).
  // These are NOT pool nodes — do not return to pool.
  while (llist_pop_front(&llist_midi_out) != NULL);
  
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
