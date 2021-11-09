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

#if defined(USB_SERIAL) || (USB_MIDI_SERIAL)
#include "serial_transmit.h"
#endif
#if defined(USB_MIDI)
#include "midi_transmit.h"
#endif
#if defined(USB_OSC)
#include "osc_transmit.h"
#endif
#if defined(MAPPING_LAYOUT)
#include "mapping.h"
#endif
#if defined(FLASH_PLAYER)
#include "player_flash.h"
#endif
#if defined(SYNTH_PLAYER)
#include "player_synth.h"
#endif
#if defined(GRANULAR_PLAYER)
#include "player_granular.h"
#endif
#if defined(FLASH_PLAYER) || (SYNTH_PLAYER) || (GRANULAR_PLAYER)
#include "soundCard.h"
#endif

unsigned long fpsTimeStamp = 0;
uint16_t fps = 0;

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

#if defined(RUNING_MEDIAN)
  RUNING_MEDIAN_SETUP();
#endif
#if defined(USB_SERIAL) || (USB_MIDI_SERIAL)
  SERIAL_TRANSMIT_SETUP();
#endif
#if defined(USB_OSC)
  OSC_TRANSMIT_SETUP();
#endif
#if defined(USB_MIDI) || (USB_MIDI_SERIAL)
  MIDI_TRANSMIT_SETUP();
#endif
#if defined(SYNTH_PLAYER)
  SYNTH_PLAYER_SETUP();
#endif
#if defined(FLASH_PLAYER)
  FLASH_PLAYER_SETUP();
#endif
#if defined(GRANULAR_PLAYER)
  GRANULAR_PLAYER_SETUP();
#endif
#if defined(MAPPING_LAYOUT)
  GRID_LAYOUT_SETUP();
  //TRIGGER_SETUP();
  //TOGGLE_SETUP();
  //VSLIDER_SETUP();
  //HSLIDER_SETUP();
  //CSLIDER_SETUP();
#endif

#if defined(SOUND_CARD)
  SOUND_CARD_SETUP();
#endif
};

void loop() {
#if defined(USB_OSC)
  read_osc_input();
#endif
#if defined(USB_MIDI) || (USB_MIDI_SERIAL)
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

#if defined(SOUND_CARD)
  update_levels();
#endif

  calibrate_matrix();
  scan_matrix();
  interp_matrix();
  find_blobs();

#if defined(MAPPING_LAYOUT)
  //mapping_grid_populate(); // Use the MIDI input messages to populate the grid - If commented we use the DEFAULT mapping
  mapping_grid_update();
  //mapping_blob();
  //mapping_trigger();
  //mapping_toggle();
  //mapping_hSlider();
  //mapping_vSlider();
  //mapping_cSlider();
#endif

#if defined(PLAYER_SYNTH)
  synth_player();
#endif
#if defined(PLAYER_GRANULAR)
  granular_player();
#endif
#if defined(PLAYER_FLASH)
  flash_player();
#endif
#if defined(USB_MIDI) || (USB_MIDI_SERIAL)
  midi_transmit();
#endif
#if defined(USB_OSC)
  osc_transmit();
#endif

  //if (loadPreset) preset_load(); // TODO
  //if (savePreset) preset_save(); // TODO

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