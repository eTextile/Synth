/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "presets.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"
#include "median.h"
#include "midi_transmit.h"

#include <elapsedMillis.h>  // https://github.com/pfeerick/elapsedMillis

#if MAPPING_LAYOUT
#include "mapping.h"
#endif

#if USB_SLIP_OSC_TRANSMIT
#include "usb_slip_osc_transmit.h"
#endif
#if MIDI_HARDWARE || MIDI_USB
#include "midi_transmit.h"
#endif

#if FLASH_PLAYER
#include "player_flash.h"
#endif
#if SYNTH_PLAYER
#include "player_synth.h"
#endif
#if GRANULAR_PLAYER
#include "player_granular.h"
#endif
#if FLASH_PLAYER || SYNTH_PLAYER || GRANULAR_PLAYER
#include "soundCard.h"
#endif

#if DEBUG_FPS
elapsedMillis curentMillisFps;
unsigned int fps = 0;
#endif

//boolean loadPreset = true;  // TODO
//boolean savePreset = false; // TODO

void setup() {
#if DEBUG_ADC || DEBUG_INTERP || DEBUG_BITMAP || DEBUG_BLOBS || DEBUG_FPS || DEBUG_ENCODER || DEBUG_BUTTONS || DEBUG_MAPPING || DEBUG_MIDI_TRANSMIT
  Serial.begin(BAUD_RATE);
  while (!Serial);
  Serial.printf("\n%s_%s_%s", NAME, PROJECT, VERSION);
  Serial.printf("\nCURRENT_MODE_:_%d", currentMode);

#endif
  LEDS_SETUP();
  SWITCHES_SETUP();
  SPI_SETUP();
  ADC_SETUP();
  SCAN_SETUP();
  INTERP_SETUP();
  BLOB_SETUP();

#if MIDI_HARDWARE || MIDI_USB
  MIDI_TRANSMIT_SETUP();
#endif

#if USB_SLIP_OSC_TRANSMIT
  USB_SLIP_OSC_SETUP();
#endif

#if SYNTH_PLAYER
  SYNTH_PLAYER_SETUP();
#endif
#if FLASH_PLAYER
  FLASH_PLAYER_SETUP();
#endif
#if GRANULAR_PLAYER
  GRANULAR_PLAYER_SETUP();
#endif
#if MAPPING_LAYOUT
  GRID_LAYOUT_SETUP();
  //TRIGGER_SETUP();
  //TOGGLES_SETUP();
  //VSLIDERS_SETUP();
  //HSLIDERS_SETUP();
  //CSLIDERS_SETUP();
#endif

#if SYNTH_PLAYER || GRANULAR_PLAYER || FLASH_PLAYER
  SOUND_CARD_SETUP();
#endif

};

void loop() {
#if MIDI_USB
  read_usb_midi_input();
#endif
#if MIDI_HARDWARE
  read_hardware_midi_input();
#endif
#if USB_SLIP_OSC
  read_usb_slip_osc_input();
#endif

  //if (loadPreset) preset_load(); // TODO
  //if (savePreset) preset_save(); // TODO

  update_buttons();
  update_presets();
  update_leds();

#if SYNTH_PLAYER || GRANULAR_PLAYER || FLASH_PLAYER
  update_levels();
#endif

  calibrate_matrix();
  scan_matrix();
  interp_matrix();
  find_blobs();
  //median();

#if MAPPING_LAYOUT
  //mapping_grid_populate(); // Use the MIDI input messages to populate the grid - If commented we use the DEFAULT mapping
  mapping_grid_update();
  //mapping_trigger();
  //mapping_toggle();
  //mapping_hSlider();
  //mapping_vSlider();
  //mapping_cSlider();
  //mapping_cChange();
#endif

#if MIDI_USB || MIDI_HARDWARE
  midi_transmit();
#endif

#if SYNTH_PLAYER
  synth_player();
#endif
#if GRANULAR_PLAYER
  granular_player();
#endif
#if FLASH_PLAYER
  flash_player();
#endif

#if DEBUG_FPS
  if (curentMillisFps >= 1000) {
    curentMillisFps = 0;
    Serial.printf("\nFPS:%d", fps);
    //Serial.printf("\nFPS:%d\tCPU:%f\tMEM:%f", fps, AudioProcessorUsageMax(), AudioMemoryUsageMax());
    //AudioProcessorUsageMaxReset();
    //AudioMemoryUsageMaxReset();
    fps = 0;
  };
  fps++;
#endif
};
