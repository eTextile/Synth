/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>

#define NAME                     "E256"
#define PROJECT                  "ETEXTILE-SYNTHESIZER"
#define VERSION                  "1.0.8"

#define VELOCITY                 0  // [0:1] IN_PROGRESS
#define RUNING_MEDIAN            0  // [0:1] IN_PROGRESS

#define SERIAL_TRANSMIT          0  // [0:1] Set the eTextile-Synthesizer as USB_MIDI divice - DO NOT FORGET: Arduino/Touls/USB_Type/Serial
#define OSC_TRANSMIT             1  // [0:1] Set the eTextile-Synthesizer as USB SLIP_OSC divice - DO NOT FORGET: Arduino/Touls/USB_Type/Serial

#define MIDI_TRANSMIT            0  // [0:1] Set the eTextile-Synthesizer as USB_MIDI divice - DO NOT FORGET: Arduino/Touls/USB_Type/MIDI
#define MIDI_HARDWARE            0  // [0:1] Set the eTextile-Synthesizer as HARDWARE_MIDI divice - DO NOT FORGET: Arduino/Touls/USB_Type/MIDI
#define MIDI_INPUT_CHANNEL       1  // [1:15] Set the HARDWARE MIDI_INPUT channel
#define MIDI_OUTPUT_CHANNEL      1  // [1:15] Set the HARDWARE MIDI_OUTPUT channel

#define SOUND_CARD               0  // [0:1] Enable DAC (Mandatory for ALL PLAYERS)
#define PLAYER_SYNTH             0  // [0:1] Set the eTextile-Synthesizer as STANDALONE divice
#define PLAYER_GRANULAR          0  // [0:1] Set the eTextile-Synthesizer as STANDALONE divice
#define PLAYER_FLASH             0  // [0:1] Set the eTextile-Synthesizer as STANDALONE divice

#define MAPPING_LAYOUT           0  // [0:1] Enable MIDI mapping functionality
#define GRID_LAYOUT_MIDI_IN      0  // [0:1] 
#define GRID_LAYOUT_DEFAULT      0  // [0:1] 
#define GRID_LAYOUT_HARMONIC     1  // [0:1] 
#define GRID_LAYOUT_NOTES        1  // [0:1] 
#define GRID_LAYOUT_FREQ         0  // [0:1] 

#define DEBUG_FPS                0  // [0:1] Print Frames Per Second
#define DEBUG_ENCODER            0  // [0:1] Print encoder value
#define DEBUG_BUTTONS            0  // [0:1] Print buttons states
#define DEBUG_ADC                0  // [0:1] Print 16x16 Analog raw values
#define DEBUG_INTERP             0  // [0:1] Print 64x64 interpolated values
#define DEBUG_BITMAP             0  // [0:1] Print 64x64 binary image based on threshold
#define DEBUG_FIND_BLOBS         0  // [0:1] Print lowlevel blobs values
#define DEBUG_BLOBS              0  // [0:1] Print blobs values
#define DEBUG_MEDIAN             0  // [0:1] Print blobs values
#define DEBUG_MAPPING            0  // [0:1] Print GUI values
#define DEBUG_MIDI_TRANSMIT      0  // [0:1] Print MIDI values
#define DEBUG_OSC_TRANSMIT       0  // [0:1] Print OSC values

#define BAUD_RATE                230400
#define RAW_COLS                 16
#define RAW_ROWS                 16
#define RAW_FRAME                (RAW_COLS * RAW_ROWS)
#define SCALE_X                  4
#define SCALE_Y                  4
#define NEW_COLS                 (RAW_COLS * SCALE_X)
#define NEW_ROWS                 (RAW_ROWS * SCALE_Y)
#define NEW_FRAME                (NEW_COLS * NEW_ROWS)
#define SIZEOF_FRAME             (NEW_FRAME * sizeof(uint8_t))

#define X_MIN                    1  // Blob centroid X min value
#define X_MAX                    58 // Blob centroid X max value
#define Y_MIN                    1  // Blob centroid Y min value
#define Y_MAX                    58 // Blob centroid Y max value

#define MAX_SYNTH                8  // [0:7] How many synthesizers can be played at the same time

//#define PI                     (float)3.1415926535897932384626433832795
#define IIPi                     (float)(2 * PI)
#define IIIPiII                  (float)(3 * PI) / 2
#define PiII                     (float)(PI / 2)

#endif /*__CONFIG_H__*/
