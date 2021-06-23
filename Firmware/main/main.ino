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

#include <elapsedMillis.h>  // https://github.com/pfeerick/elapsedMillis

#if MAPPING_LAYAOUT
#include "mapping.h"
#endif

#if USB_MIDI_TRANSMIT
#include "usb_midi_transmit.h"
#endif
#if USB_SLIP_OSC_TRANSMIT
#include "usb_slip_osc_transmit.h"
#endif
#if HARDWARE_MIDI_TRANSMIT
#include "hardware_midi_transmit.h"
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

//boolean loadPreset = true;
//boolean savePreset = false;

hSlider_t hSliderParam = {30, 15, 40, 5, 0};        // ARGS[posY, Xmin, Xmax, width, val]
cChange_t ccParam = {NULL, BD, 44, 1, 0};           // ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]

void setup() {
#if DEBUG_ADC || DEBUG_INTERP || DEBUG_BITMAP || DEBUG_BLOBS || DEBUG_FPS || DEBUG_ENCODER || DEBUG_BUTTONS || DEBUG_MAPPING
  Serial.begin(BAUD_RATE); // Start Serial communication using 230400 baud
  while (!Serial);
  Serial.printf("\n%s_%s", NAME, VERSION);
#endif
  LEDS_SETUP();
  SWITCHES_SETUP();
  SPI_SETUP();
  ADC_SETUP();
  SCAN_SETUP();
  INTERP_SETUP();
  BLOB_SETUP();

#if USB_MIDI_TRANSMIT
  TRANSMIT_SETUP();
  USB_MIDI_SETUP();
#endif
#if USB_SLIP_OSC_TRANSMIT
  TRANSMIT_SETUP();
  USB_SLIP_OSC_SETUP();
#endif
#if HARDWARE_MIDI_TRANSMIT
  TRANSMIT_SETUP();
  HARDWARE_MIDI_SETUP();
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
#if MAPPING_LAYAOUT
  GRID_LAYOUT_SETUP();
  TOGGLES_SETUP();
  TRIGGERS_SETUP();
  VSLIDERS_SETUP();
  HSLIDERS_SETUP();
#endif

#if SYNTH_PLAYER || GRANULAR_PLAYER || FLASH_PLAYER
  SOUND_CARD_SETUP();
#endif
};

void loop() {
  //if (loadPreset) preset_load(); // TODO
  //if (savePreset) preset_save(); // TODO

  update_buttons();
  update_presets();

#if SYNTH_PLAYER || GRANULAR_PLAYER || FLASH_PLAYER
  update_levels();
#endif

  update_leds();
  calibrate_matrix();
  scan_matrix();
  interp_matrix();
  find_blobs();
  //median();

#if USB_MIDI_TRANSMIT
  usb_midi_handle_input();
#endif
#if USB_SLIP_OSC_TRANSMIT
  usb_slip_osc_handle_input();
#endif
#if HARDWARE_MIDI_TRANSMIT
  hardware_midi_handle_input();
#endif

#if MAPPING_LAYAOUT
  mapping_gridPlay();
  //mapping_toggles();
  //mapping_triggers();
  //mapping_hSliders();
  //mapping_vSliders();
  //mapping_cChange(&ccParam);
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
