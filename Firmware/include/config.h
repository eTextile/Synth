/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <ArduinoJson.h>

#define NAME                       "256"
#define PROJECT                    "ETEXTILE-SYNTHESIZER"
#define VERSION                    "1.0.9"

// State machine
#define STATE_START                0
#define STATE_SIZE                 1
#define STATE_CONTENT              2
// Special bytes in the communication protocol
#define HAND_SHAKE                 0x7f
#define BYTE_START                 0x7e
#define BYTE_ESCAPE                0x7d
#define BYTE_SEPARATOR             0x7c

#define LOAD_CONFIG                0  // E256-LEDs: 
#define FLASH_CONFIG               1  // E256-LEDs:
#define CALIBRATE                  2  // E256-LEDs: 
#define BLOBS_PLAY                 3  // Send all blobs values over USB using MIDI format
#define BLOBS_LEARN                4  // Send separate blobs values over USB using MIDI format
#define MAPPING_LIB                5  // E256-LEDs:
#define RAW_MATRIX                 6
#define INTERP_MATRIX              7
#define ERROR                      6  // E256-LEDs:

#define ALL_OFF                    9

#define SIG_IN                     0  // E256-LEDs: | 1 | 0 |
#define SIG_OUT                    1  // E256-LEDs: | 0 | 1 |
#define LINE_OUT                   2  // E256-LEDs: | 0 | 0 |
#define THRESHOLD                  3  // E256-LEDs: | 1 | 1 |

// E256 HARDWARE CONSTANTES **DO NOT CHANGE**
#if defined(__IMXRT1062__)
#define LED_PIN_D1                  5
#define LED_PIN_D2                  4
#define BUTTON_PIN_L                2
#define BUTTON_PIN_R                3
#define ENCODER_PIN_A               22
#define ENCODER_PIN_B               9
#define FLASH_CHIP_SELECT           6
#define FLASH_BUFFER_SIZE           4096
#define FILENAME_STRING_SIZE        11 // config.json
#endif

#define BAUD_RATE                   230400
#define RAW_COLS                    16
#define RAW_ROWS                    16
#define RAW_FRAME                   (RAW_COLS * RAW_ROWS)
#define SCALE_X                     4
#define SCALE_Y                     4
#define NEW_COLS                    (RAW_COLS * SCALE_X)
#define NEW_ROWS                    (RAW_ROWS * SCALE_Y)
#define NEW_FRAME                   (NEW_COLS * NEW_ROWS)
#define SIZEOF_FRAME                (NEW_FRAME * sizeof(uint8_t))

#define BLOB_MIN_PIX                6     // Set the minimum blob pixels
#define BLOB_MAX_PIX                1024  // Set the minimum blob pixels

#define X_MIN                       1     // Blob centroid X min value
#define X_MAX                       58    // Blob centroid X max value
#define Y_MIN                       1     // Blob centroid Y min value
#define Y_MAX                       58    // Blob centroid Y max value
#define WIDTH                       (X_MAX - X_MIN)
#define HEIGHT                      (Y_MAX - Y_MIN)

#define MAX_BLOBS                   8     // [0:7] How many blobs can be tracked at the same time
#define MAX_TRIGGERS                16
#define MAX_TOGGLES                 16
#define MAX_HSLIDERS                8
#define MAX_VSLIDERS                8
#define MAX_CIRCLES                 9
#define MAX_TOUCHPADS               4
#define MAX_VERTICES                8
#define MAX_POLYGONS                8

//#define PI                        (float)3.1415926535897932384626433832795
#define IIPi                        (float)(2 * PI)
#define IIIPiII                     (float)(3 * PI) / 2
#define PiII                        (float)(PI / 2)

#define MIDI_INPUT_CHANNEL          1  // [1:15] Set the HARDWARE MIDI_INPUT channel
#define MIDI_OUTPUT_CHANNEL         1  // [1:15] Set the HARDWARE MIDI_OUTPUT channel

extern uint8_t currentMode;   // Exposed local declaration see config.cpp
extern uint8_t currentLevel;

typedef struct leds leds_t;
struct leds {
  boolean D1;
  boolean D2;
  boolean setup;
  boolean update;
};

typedef struct e256_mode e256_mode_t;
struct e256_mode {
  leds_t leds;
  uint16_t timeOn;
  uint16_t timeOff;
  boolean toggle;
  boolean run;
};

typedef struct e256_level e256_level_t;
struct e256_level {
  leds_t leds;
  uint8_t minVal;
  uint8_t maxVal;
  uint8_t val;
  boolean run;
};

extern e256_level_t levels[];  // Exposed local declaration see config.cpp
extern e256_mode_t modes[];  // Exposed local declaration see config.cpp

void CONFIG_SETUP(void);
bool config_load_mapping(const JsonObject &config);
void update_config(void);

#endif /*__CONFIG_H__*/