/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>

#define NAME                     "256"
#define PROJECT                  "ETEXTILE-SYNTHESIZER"
#define VERSION                  "1.0.9"

// HARDWARE CONSTANTES **DO NOT CHANGE**
#if defined(__IMXRT1062__)
#define LED_PIN_D1              5
#define LED_PIN_D2              4
#define BUTTON_PIN_L            2
#define BUTTON_PIN_R            3
#define ENCODER_PIN_A           22
#define ENCODER_PIN_B           9
#endif

//#define BAUD_RATE                230400
#define BAUD_RATE                9600
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
#define WIDTH                    (X_MAX - X_MIN)
#define HEIGHT                   (Y_MAX - Y_MIN)

#define MAX_BLOBS                8  // [0:7] How many blobs can be tracked at the same time

//#define PI                     (float)3.1415926535897932384626433832795
#define IIPi                     (float)(2 * PI)
#define IIIPiII                  (float)(3 * PI) / 2
#define PiII                     (float)(PI / 2)

#define MIDI_INPUT_CHANNEL       1  // [1:15] Set the HARDWARE MIDI_INPUT channel
#define MIDI_OUTPUT_CHANNEL      1  // [1:15] Set the HARDWARE MIDI_OUTPUT channel

#endif /*__CONFIG_H__*/