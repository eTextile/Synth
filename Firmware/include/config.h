/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <ArduinoJson.h>
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>                   // https://github.com/PaulStoffregen/Encoder

#define NAME                         "256"
#define PROJECT                      "ETEXTILE-SYNTHESIZER"
#define VERSION                      "1.0.9"
#define SENSOR_UID                   1 // To add into the .json config file

#define LOAD_CONFIG                  0  // E256-LEDs: 
#define FLASH_CONFIG                 1  // E256-LEDs:
#define CALIBRATE                    2  // E256-LEDs: 
#define BLOBS_PLAY                   3  // Send all blobs values over USB using MIDI format
#define BLOBS_LEARN                  4  // Send separate blobs values over USB using MIDI format
#define MAPPING_LIB                  5  // E256-LEDs:
#define RAW_MATRIX                   6
#define INTERP_MATRIX                7
#define ERROR                        6  // E256-LEDs:

#define ALL_OFF                      9

#define SIG_IN                       0  // E256-LEDs: | 1 | 0 |
#define SIG_OUT                      1  // E256-LEDs: | 0 | 1 |
#define LINE_OUT                     2  // E256-LEDs: | 0 | 0 |
#define THRESHOLD                    3  // E256-LEDs: | 1 | 1 |

// E256 HARDWARE CONSTANTES **DO NOT CHANGE**
#define LED_PIN_D1                   5
#define LED_PIN_D2                   4

#define FLASH_CHIP_SELECT            6
#define FLASH_SIZE                   4096
#define FILENAME_STRING_SIZE         11 // config.json

#define MAPPING_CONFIG               127
#define AUDIO_FILE                   128

#define BAUD_RATE                    230400
#define RAW_COLS                     16
#define RAW_ROWS                     16
#define RAW_FRAME                    (RAW_COLS * RAW_ROWS)
#define SCALE_X                      4
#define SCALE_Y                      4
#define NEW_COLS                     (RAW_COLS * SCALE_X)
#define NEW_ROWS                     (RAW_ROWS * SCALE_Y)
#define NEW_FRAME                    (NEW_COLS * NEW_ROWS)
#define SIZEOF_FRAME                 (NEW_FRAME * sizeof(uint8_t))

#define BLOB_MIN_PIX                 6     // Set the minimum blob pixels
#define BLOB_MAX_PIX                 1024  // Set the minimum blob pixels

#define X_MIN                        1     // Blob centroid X min value
#define X_MAX                        58    // Blob centroid X max value
#define Y_MIN                        1     // Blob centroid Y min value
#define Y_MAX                        58    // Blob centroid Y max value
#define WIDTH                        (X_MAX - X_MIN)
#define HEIGHT                       (Y_MAX - Y_MIN)

#define MAX_BLOBS                    8     // [0:7] How many blobs can be tracked at the same time
#define MAX_TRIGGERS                 16
#define MAX_TOGGLES                  16
#define MAX_HSLIDERS                 8
#define MAX_VSLIDERS                 8
#define MAX_CIRCLES                  9
#define MAX_TOUCHPADS                4
#define MAX_VERTICES                 8
#define MAX_POLYGONS                 8

//#define PI                         (float)3.1415926535897932384626433832795
#define IIPi                         (float)(2 * PI)
#define IIIPiII                      (float)(3 * PI) / 2
#define PiII                         (float)(PI / 2)

#define MIDI_INPUT_CHANNEL           1  // [1:15] Set the HARDWARE MIDI_INPUT channel
#define MIDI_OUTPUT_CHANNEL          1  // [1:15] Set the HARDWARE MIDI_OUTPUT channel

#define ERROR_WAITING_FOR_GONFIG     33
#define ERROR_LOADING_GONFIG_FAILED  34
#define ERROR_CONNECTING_FLASH       35
#define ERROR_WHILE_OPEN_FLASH_FILE  36
#define ERROR_FLASH_FULL             37
#define ERROR_FILE_TO_BIG            38
#define ERROR_NO_CONFIG_FILE         39

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

typedef struct e256_control e256_control_t;
struct e256_control {
  e256_mode_t* modes;
  e256_level_t* levels;
};

extern Encoder e256_encoder;
extern e256_control_t e256_ctr;

extern  uint8_t playMode;
extern  uint8_t lastMode;
extern  uint8_t levelMode;
 
extern  uint32_t ledsTimeStamp;
extern  uint8_t ledsIterCount;

void CONFIG_SETUP(void);
void setup_leds(void* struct_ptr);

void set_mode(e256_control* control, uint8_t mode);
void set_level(e256_control* control, uint8_t level);

boolean load_config(char* data_ptr);
void update_config(void);

#endif /*__CONFIG_H__*/