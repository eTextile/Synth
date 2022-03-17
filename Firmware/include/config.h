/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>                 // https://github.com/PaulStoffregen/Encoder

#define NAME                         "256"
#define PROJECT                      "ETEXTILE-SYNTHESIZER"
#define VERSION                      "1.0.10"
#define SENSOR_UID                   1 // Unique sensor ID

// E256 HARDWARE CONSTANTS
#define LED_PIN_D1                   5
#define LED_PIN_D2                   4
#define BUTTON_PIN_L                 2
#define BUTTON_PIN_R                 3
#define ENCODER_PIN_A                22
#define ENCODER_PIN_B                9
#define FLASH_CHIP_SELECT            6
#define FLASH_SIZE                   4096
#define BAUD_RATE                    230400
#define RAW_COLS                     16
#define RAW_ROWS                     16
#define RAW_FRAME                    (RAW_COLS * RAW_ROWS)

// E256 SOFTWARE CONSTANTS
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
#define IIPi                         (float)(2 * PI)
#define IIIPiII                      (float)(3 * PI) / 2
#define PiII                         (float)(PI / 2)
#define LONG_HOLD                    1500
#define BLINK_ITER                   10
#define MIDI_TRANSMIT_INTERVAL       50

// E256 MODES CONSTANTS
#define MATRIX_MODE_RAW              0 // Send matrix analog sensor values (16x16) over USB using MIDI format
#define MATRIX_MODE_INTERP           1 // Send matrix analog sensor values (16x16) over USB using MIDI format
#define STANDALONE_MODE              2 // Send mappings values over MIDI hardware
#define EDIT_MODE                    3 // Send all blobs values over USB_MIDI
#define PLAY_MODE                    4 // Send mappings values over USB_MIDI

// E256 STATES CONSTANTS
#define CALIBRATE                    0 //
#define CONFIG                       1 //
#define DONE_ACTION                  2 // 
#define ERROR                        3 //

// E256 LEVELS CONSTANTS
#define THRESHOLD                    0 // E256-LEDs: | 1 | 1 |
#define SIG_IN                       1 // E256-LEDs: | 1 | 0 |
#define SIG_OUT                      2 // E256-LEDs: | 0 | 1 |
#define LINE_OUT                     3 // E256-LEDs: | 0 | 0 |

// E256 MAPPING_LIB CONSTANTS
#define MAX_BLOBS                    32    // [0:7] How many blobs can be tracked at the same time
#define MAX_TRIGGERS                 16
#define MAX_TOGGLES                  16
#define MAX_HSLIDERS                 8
#define MAX_VSLIDERS                 8
#define MAX_CIRCLES                  9
#define MAX_TOUCHPADS                4
#define MAX_POLYGON_POINT            128
#define MAX_POLYGONS                 8

// E256 MIDI CONSTANTS
#define MIDI_INPUT_CHANNEL           1  // [1:15] Set the HARDWARE MIDI_INPUT channel
#define MIDI_OUTPUT_CHANNEL          1  // [1:15] Set the HARDWARE MIDI_OUTPUT channel
#define SYSEX_CONF                   0x7C
#define SYSEX_SOUND                  0x6C

// VERBOSITY CONSTANTS
#define DONE_FLASH_CONFIG_ALLOC      16
#define DONE_FLASH_CONFIG_LOAD       17
#define DONE_FLASH_CONFIG_WRITE      18
#define DONE_USBMIDI_CONFIG_ALLOC    19
#define DONE_USBMIDI_CONFIG_LOAD     20
#define DONE_USBMIDI_SOUND_LOAD      21

// ERROR CODES CONSTANTS
#define ERROR_WAITING_FOR_GONFIG     33
#define ERROR_LOADING_GONFIG_FAILED  34
#define ERROR_CONNECTING_FLASH       35
#define ERROR_WHILE_OPEN_FLASH_FILE  36
#define ERROR_FLASH_FULL             37
#define ERROR_FILE_TO_BIG            38
#define ERROR_NO_CONFIG_FILE         39
#define ERROR_UNKNOWN_SYSEX          40

typedef struct leds leds_t;
struct leds {
  boolean D1;
  boolean D2;
  boolean update;
};

typedef struct e256_mode e256_mode_t;
struct e256_mode {
  leds_t leds;
  uint16_t timeOn;
  uint16_t timeOff;
  boolean toggle;
};

typedef struct e256_state e256_state_t;
struct e256_state {
  leds_t leds;
  uint16_t timeOn;
  uint16_t timeOff;
  uint8_t iter;
};

typedef struct e256_level e256_level_t;
struct e256_level {
  leds_t leds;
  uint8_t minVal;
  uint8_t maxVal;
  uint8_t val;
  boolean update;
};

extern uint16_t configSize;

typedef struct e256_control e256_control_t;
struct e256_control {
  Encoder* encoder;
  e256_mode_t* modes;
  e256_state_t* states;
  e256_level_t* levels;
};

extern e256_control_t e256_ctr;

extern uint8_t e256_mode;
extern uint8_t e256_last_mode;
extern uint8_t e256_level;

void config_setup(void);

void set_mode(uint8_t mode);
void set_level(uint8_t level, uint8_t value);
void set_state(uint8_t state);

void load_config(uint8_t* data_ptr, uint8_t msg);
void update_controls(void);

#endif /*__CONFIG_H__*/