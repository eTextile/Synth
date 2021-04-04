/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>

//#if defined(USB_SERIAL)
//#if defined(USB_MIDI)

#define NAME                    "ETEXTILE_SYNTHESIZER"
#define VERSION                 "1.0.2"

#define USB_MIDI                0  // [0:1] Set the eTextile-Synthesizer as USB MIDI divice
#define USB_SLIP_OSC            0  // [0:1] Set the eTextile-Synthesizer as SLIP_OSC divice
#define HARDWARE_MIDI           0  // [0:1] Set the eTextile-Synthesizer as MIDI I/O divice

#define SYNTH_PLAYER            0  // [0:1] Set the eTextile-Synthesizer as STANDALONE divice
#define GRANULAR_PLAYER         0  // [0:1] Set the eTextile-Synthesizer as STANDALONE divice
#define FLASH_PLAYER            0  // [0:1] Set the eTextile-Synthesizer as STANDALONE divice

#define MAX_BLOBS               16 // [1:8] Set how many blobs can be tracked at the same time
#define MAX_SYNTH               8  // [1:8] Set how many synthesizers can play at the same time

// Arduino serial monitor
#define DEBUG_FPS               0  // [0:1] Print Frames Per Second
#define DEBUG_ENCODER           0  // [0:1] Print encoder value 
#define DEBUG_BUTTONS           0  // [0:1] Print buttons states
#define DEBUG_ADC               0  // [0:1] Print 16x16 Analog raw values
#define DEBUG_INTERP            0  // [0:1] Print 64x64 interpolated values
#define DEBUG_BITMAP            0  // [0:1] Print 64x64 binary image based on threshold
#define DEBUG_BLOBS             0  // [0:1] Print blobs values
#define DEBUG_MAPPING           0  // [0:1] Print blobs values

#define DEBUG_BLOBS_ID          0 // [0:1] Print lowlevel blobs values
#define DEBUG_SORT              0 // [0:1] Print lowlevel blobs sort (NOT_USED)

#define SET_ORIGIN_X            1 // [-1:1] X-axis origine positioning
#define SET_ORIGIN_Y            1 // [-1:1] Y-axis origine positioning

// DO NOT CHANGE
#define LED_PIN_D1              5
#define LED_PIN_D2              4
#define BUTTON_PIN_L            2
#define BUTTON_PIN_R            3
#define ENCODER_PIN_A           22
#define ENCODER_PIN_B           9

#define SS1_PIN                 0       // Teensy4.0 hardware SPI1 (SELECT : STCP)
#define SCK1_PIN                27      // Teensy4.0 hardware SPI1 (CLOCK - SHCP)
#define MOSI1_PIN               26      // Teensy4.0 hardware SPI1 (DATA - DS)
#define ADC0_PIN                A3      // The output of multiplexerA (SIG pin) is connected to Teensy 4.0 pin 16
#define ADC1_PIN                A2      // The output of multiplexerB (SIG pin) is connected to Teensy 4.0 pin 17

#define MEM_MOSI                11      // Teensy4.0 hardware SPI
#define MEM_MISO                12      // Teensy4.0 hardware SPI
#define MEM_SCK                 13      // Teensy4.0 hardware SPI
#define MEM_SC                  6       // Teensy4.0 hardware SPI

#define BAUD_RATE               230400
#define RAW_COLS                16
#define DUAL_COLS               (RAW_COLS / 2)
#define RAW_ROWS                16
#define SCALE_X                 4
#define SCALE_Y                 4
#define RAW_FRAME               (RAW_COLS * RAW_ROWS)
#define NEW_COLS                (RAW_COLS * SCALE_X)
#define NEW_ROWS                (RAW_ROWS * SCALE_Y)
#define NEW_FRAME               (NEW_COLS * NEW_ROWS)
#define POLAR_CX                (NEW_COLS / 2)
#define POLAR_CY                (NEW_ROWS / 2)
#define LIFO_MAX_NODES          255      // Set the maximum nodes number
#define X_STRIDE                3        // 
#define Y_STRIDE                3        // 
#define MIN_BLOB_PIX            8        // Set the minimum blob pixels
#define MAX_BLOB_PIX            4095     // Set the maximum blob pixels
#define CALIBRATION_CYCLES      10

#define DEBOUNCE_TIME_BLOB      15
#define DEBOUNCE_TIME_SWITCH    15

#define PI                      3.1415926535897932384626433832795
#define PI2                     (PI+PI)

#define C_SLIDERS               3

#define X_MAX                   58       // Blob X_MAX centroid position
#define Y_MAX                   58       // Blob Y_MAX centroid position

#define GRID_COLS               8
#define GRID_ROWS               8
#define GRID_KEYS               (GRID_COLS * GRID_ROWS)
#define GRID_GAP                2
#define KEY_SIZE                ((float)((X_MAX - ((GRID_COLS + 1) * GRID_GAP)) / GRID_COLS))

#define MAX_BLOB_PARAM          7        // [BI, BS, BX, BY, BW, BH, BD]
#define BI                      0        // Blob UID
#define BS                      1        // Blob State
#define BX                      2        // Blob Centroid PosX
#define BY                      3        // Blob Centroid PosY
#define BW                      4        // Blob width
#define BH                      5        // Blob Height
#define BD                      6        // Blob Depth 

#define LONG_HOLD               1500
#define MIDI_LEARN_LED_TIMEON   600
#define MIDI_LEARN_LED_TIMEOFF  600
#define CALIBRATE_LED_TIMEON    35
#define CALIBRATE_LED_TIMEOFF   100
#define CALIBRATE_LED_ITER      4
#define SAVE_LED_TIMEON         20
#define SAVE_LED_TIMEOFF        50
#define SAVE_LED_ITER           10

#define GRANULAR_MEMORY_SIZE    12800

#endif /*__CONFIG_H__*/
