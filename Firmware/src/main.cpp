/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

//#include "config.h"
//#include "presets.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"

#if defined(USB_MTPDISK) || (USB_MTPDISK_MIDI)
#include "mtp_spi.h"
#include "json_parser.h"
#include "usb_midi_transmit.h"
//#include "usb_osc_transmit.h"
#endif
#if defined(USB_SERIAL) || (USB_MIDI_SERIAL)
#include "usb_serial_transmit.h"
#endif
#if defined(USB_MIDI)
#include "usb_midi_transmit.h"
#endif
#if defined(USB_OSC)
#include "usb_osc_transmit.h"
#endif
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
#if defined(PLAYER_FLASH) || (PLAYER_SYNTH) || (PLAYER_GRANULAR)
#include "soundCard.h"
#endif

unsigned long fpsTimeStamp = 0;
uint16_t fps = 0;

void setup() {

  LEDS_SETUP();
  SWITCHES_SETUP();
  SPI_SETUP();
  ADC_SETUP();
  SCAN_SETUP();
  INTERP_SETUP();
  BLOB_SETUP();
  MIDI_NODES_SETUP();

#if defined(USB_MTPDISK) || (USB_MTPDISK_MIDI)
  MTP_SPI_SETUP();
  JSON_PARSER_SETUP();
#endif
#if defined(RUNING_MEDIAN)
  RUNING_MEDIAN_SETUP();
#endif
#if defined(USB_SERIAL) || (USB_MIDI_SERIAL)
  USB_SERIAL_TRANSMIT_SETUP();
#endif
#if defined(USB_MIDI) || (USB_MIDI_SERIAL) || (USB_MTPDISK_MIDI)
  USB_MIDI_TRANSMIT_SETUP();
#endif
#if defined(USB_OSC)
  USB_OSC_TRANSMIT_SETUP();
#endif
#if defined(HARDWARE_MIDI)
  HARDWARE_MIDI_TRANSMIT_SETUP();
#endif


#if defined(PLAYER_SYNTH)
  PLAYER_SYNTH_SETUP();
#endif
#if defined(PLAYER_FLASH)
  PLAYER_FLASH_SETUP();
#endif
#if defined(PLAYER_GRANULAR)
  PLAYER_GRANULAR_SETUP();
#endif
#if defined(MAPPING_LAYOUT)
  MAPPING_LAYOUT_SETUP();
#endif
#if defined(SOUND_CARD)
  SOUND_CARD_SETUP();
#endif
};

void loop() {
#if defined(USB_MTPDISK) || (USB_MTPDISK_MIDI)
  handle_mtp_spi();
#endif
#if defined(USB_MIDI) || (USB_MIDI_SERIAL) || (USB_MTPDISK_MIDI)
  usb_midi_read_input();
#endif
#if defined(USB_OSC)
  usb_osc_read_input();
#endif
#if defined(HARDWARE_MIDI)
  hardware_midi_read_input();
#endif
#if defined(__MK20DX256__)  // Teensy 3.1 & 3.2
  update_presets_midi_usb();
#endif
#if defined(__IMXRT1062__)  // Teensy 4.0 & 4.1
  update_presets_usb();
  update_presets_buttons();
  update_presets_encoder();
  update_leds();
#endif
#if defined(SOUND_CARD)
  update_levels();
#endif
  matrix_calibrate();
  matrix_scan();
  matrix_interp();
  matrix_find_blobs();
#if defined(MAPPING_LAYOUT)
  mapping_layout_update();
#endif
#if defined(PLAYER_SYNTH)
  player_synth();
#endif
#if defined(PLAYER_GRANULAR)
  player_granular();
#endif
#if defined(PLAYER_FLASH)
  player_flash();
#endif
#if defined(USB_MIDI) || (USB_MIDI_SERIAL) || (USB_MTPDISK_MIDI)
  usb_midi_transmit();
#endif
#if defined(USB_OSC)
  usb_osc_transmit();
#endif
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