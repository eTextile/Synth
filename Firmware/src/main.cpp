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

unsigned long fpsTimeStamp = 0;

#if USB_SERIAL_TRANSMIT
#include "serial_transmit.h"
#endif
#if USB_MIDI_TRANSMIT
#include "midi_transmit.h"
#endif
#if USB_OSC_TRANSMIT
#include "osc_transmit.h"
#endif
#if MAPPING_LAYOUT
#include "mapping.h"
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

  LEDS_SETUP();
  SWITCHES_SETUP();
  SPI_SETUP();
  ADC_SETUP();
  SCAN_SETUP();
  INTERP_SETUP();
  BLOB_SETUP();

#if RUNING_MEDIAN
  RUNING_MEDIAN_SETUP();
#endif
#if SERIAL_TRANSMIT
  SERIAL_TRANSMIT_SETUP();
#endif
#if OSC_TRANSMIT
  OSC_TRANSMIT_SETUP();
#endif
#if MIDI_TRANSMIT
  MIDI_TRANSMIT_SETUP();
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
  //TOGGLE_SETUP();
  //VSLIDER_SETUP();
  //HSLIDER_SETUP();
  //CSLIDER_SETUP();
#endif

#if SOUND_CARD
  SOUND_CARD_SETUP();
#endif

};

void loop() {
#if OSC_TRANSMIT
  read_osc_input();
#endif
#if MIDI_TRANSMIT
  read_midi_input();
#endif

#if defined(__MK20DX256__)     // If using Teensy 3.1 & 3.2
  update_presets_midi_usb();
#endif
#if defined(__IMXRT1062__)     // If using Teensy 4.0 & 4.1
  update_presets_usb();
  update_presets_buttons();
  update_presets_encoder();
  update_leds();
#endif

#if SOUND_CARD
  update_levels();
#endif

  calibrate_matrix();
  scan_matrix();
  interp_matrix();
  find_blobs();

#if MAPPING_LAYOUT
  //mapping_grid_populate(); // Use the MIDI input messages to populate the grid - If commented we use the DEFAULT mapping
  mapping_grid_update();
  //mapping_blob();
  //mapping_trigger();
  //mapping_toggle();
  //mapping_hSlider();
  //mapping_vSlider();
  //mapping_cSlider();
#endif

#if PLAYER_SYNTH
  synth_player();
#endif
#if PLAYER_GRANULAR
  granular_player();
#endif
#if PLAYER_FLASH
  flash_player();
#endif
#if MIDI_TRANSMIT
  midi_transmit();
#endif
#if OSC_TRANSMIT
  osc_transmit();
#endif

  //if (loadPreset) preset_load(); // TODO
  //if (savePreset) preset_save(); // TODO

#if DEBUG_FPS
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
