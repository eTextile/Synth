/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <Bounce2.h>                   // https://github.com/thomasfredericks/Bounce2
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>                   // https://github.com/PaulStoffregen/Encoder

#define NAME                    "256"
#define PROJECT                 "ETEXTILE-SYNTHESIZER"
#define VERSION                 "1.0.9"

#define LINE_OUT              0  // LED | 0 | 0 |
#define SIG_IN                1  // LED | 1 | 0 |
#define SIG_OUT               2  // LED | 0 | 1 |
#define THRESHOLD             3  // LED | 1 | 1 |
#define CALIBRATE             4
#define SAVE                  5  // Not used!
#define BLOBS_PLAY            6  // Send all blobs values over USB using MIDI format
#define BLOBS_LEARN           7  // Send separate blobs values over USB using MIDI format
#define MAPPING_LIB           8
#define RAW_MATRIX            9
#define INTERP_MATRIX         10
#define ALL_OFF               11

#define LOAD_CONFIG           12
#define UPDATE_CONFIG         13

// SOFTWARE CONSTANTES **DO NOT CHANGE**
#define LONG_HOLD                   1500
#define BLOBS_PLAY_LED_TIMEON       600
#define BLOBS_PLAY_LED_TIMEOFF      600
#define BLOBS_LEARN_LED_TIMEON      100
#define BLOBS_LEARN_LED_TIMEOFF     100
#define BLOBS_MAPPING_LED_TIMEON    1000
#define BLOBS_MAPPING_LED_TIMEOFF   100
#define CALIBRATE_LED_TIMEON        35
#define CALIBRATE_LED_TIMEOFF       100
#define CALIBRATE_LED_ITER          5

#define UPDATE_CONFIG_LED_TIMEON    20
#define UPDATE_CONFIG_LED_TIMEOFF   50
#define UPDATE_CONFIG_LED_ITER      10

// HARDWARE CONSTANTES **DO NOT CHANGE**
#if defined(__IMXRT1062__)
#define LED_PIN_D1              5
#define LED_PIN_D2              4
#define BUTTON_PIN_L            2
#define BUTTON_PIN_R            3
#define ENCODER_PIN_A           22
#define ENCODER_PIN_B           9
#endif

//#define BAUD_RATE             9600
#define BAUD_RATE               230400
#define RAW_COLS                16
#define RAW_ROWS                16
#define RAW_FRAME               (RAW_COLS * RAW_ROWS)
#define SCALE_X                 4
#define SCALE_Y                 4
#define NEW_COLS                (RAW_COLS * SCALE_X)
#define NEW_ROWS                (RAW_ROWS * SCALE_Y)
#define NEW_FRAME               (NEW_COLS * NEW_ROWS)
#define SIZEOF_FRAME            (NEW_FRAME * sizeof(uint8_t))

#define BLOB_MIN_PIX            6     // Set the minimum blob pixels
#define BLOB_MAX_PIX            1024  // Set the minimum blob pixels

#define X_MIN                   1     // Blob centroid X min value
#define X_MAX                   58    // Blob centroid X max value
#define Y_MIN                   1     // Blob centroid Y min value
#define Y_MAX                   58    // Blob centroid Y max value
#define WIDTH                   (X_MAX - X_MIN)
#define HEIGHT                  (Y_MAX - Y_MIN)

#define MAX_BLOBS               8     // [0:7] How many blobs can be tracked at the same time
#define MAX_TOUCHPADS           4
#define MAX_TRIGGERS            16
#define MAX_TOGGLES             16
#define MAX_CIRCLES             9
#define MAX_VERTICES            8
#define MAX_POLYGONS            8
#define MAX_VSLIDERS            8
#define MAX_HSLIDERS            8

//#define PI                     (float)3.1415926535897932384626433832795
#define IIPi                     (float)(2 * PI)
#define IIIPiII                  (float)(3 * PI) / 2
#define PiII                     (float)(PI / 2)

#define MIDI_INPUT_CHANNEL       1  // [1:15] Set the HARDWARE MIDI_INPUT channel
#define MIDI_OUTPUT_CHANNEL      1  // [1:15] Set the HARDWARE MIDI_OUTPUT channel

#define FLASH_CHIP_SELECT       6

//////////////////////////////////////// WRITE CONFIG
//Using: /Synth/Software/Python/usb_config/rawfile-uploader.py
// $ python rawfile-uploader.py <port> config.json

// Buffer sizes
#define USB_BUFFER_SIZE       128
#define FLASH_BUFFER_SIZE     4096
// Max filename length (8.3 plus a null char terminator)
#define FILENAME_STRING_SIZE  11
// State machine
#define STATE_START           0
#define STATE_SIZE            1
#define STATE_CONTENT         2
// Special bytes in the communication protocol
#define BYTE_START            0x7e
#define BYTE_ESCAPE           0x7d
#define BYTE_SEPARATOR        0x7c

#define SERIAL_UPDATE_CONFIG_TIMEOUT 15000

extern Encoder encoder;       // Exposed local declaration see presets.cpp
extern uint8_t currentMode;   // Exposed local declaration see presets.cpp
extern uint8_t lastMode;      // Exposed local declaration see presets.cpp

typedef struct preset preset_t;
struct preset {
  uint8_t minVal;
  uint8_t maxVal;
  uint8_t val;
  uint8_t ledVal;
  boolean update;
  boolean setLed;
  boolean updateLed;
  boolean allDone;
  boolean D1;
  boolean D2;
};

extern preset_t presets[];        // Exposed local declaration see presets.cpp

void LEDS_SETUP(void);
void SWITCHES_SETUP(void);

void update_presets_usb_midi(void);
void update_presets_usb_serial(void);

void update_presets_buttons(void);
void update_presets_encoder(void);
void update_leds(void);

boolean setLevel(preset_t* preset_ptr);

void usb_serial_update_config(void);
void LOAD_SPI_FLASH_CONFIG(void);

#endif /*__CONFIG_H__*/