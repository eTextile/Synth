/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#define ENCODER_USE_INTERRUPTS
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h> // https://github.com/PaulStoffregen/Encoder

#define PROJECT "ETEXTILE-SYNTHESIZER"
#define NAME "256"
#define VERSION "1.0.27"
#define SENSOR_UID 1 // Unique sensor ID 
#define FLASH_BUFFER_SIZE 4096

// E256 HARDWARE CONSTANTS
#define LED_PIN_D1 5
#define LED_PIN_D2 4
#define BUTTON_PIN_L 2
#define BUTTON_PIN_R 3
#define ENCODER_PIN_A 22
#define ENCODER_PIN_B 9
#define FLASH_CHIP_SELECT 6
#define FLASH_CHIP_SIZE 16777216 // 128 Mb
#define BAUD_RATE 230400
#define RAW_COLS 16
#define RAW_ROWS 16
#define RAW_FRAME (RAW_COLS * RAW_ROWS)

// E256 SOFTWARE CONSTANTS
#define SCALE_X 4
#define SCALE_Y 4
#define NEW_COLS (RAW_COLS * SCALE_X)
#define NEW_ROWS (RAW_ROWS * SCALE_Y)
#define NEW_FRAME (NEW_COLS * NEW_ROWS)
#define SIZEOF_FRAME (NEW_FRAME * sizeof(uint8_t))

#define BLOB_MIN_PIX 6         // Set the minimum blob pixels
#define BLOB_MAX_PIX 1024      // Set the maximum blob pixels

#define BLOB_LAST_DIST 7       // This is the minimum distance used to identify two corresponding blobs contains int two sequential frames
#define BLOB_MISSING_TIME 50   // After this amount of time the blob is set to RELEASED
#define BLOB_RELEASE_TIME 500  // After this amount of time the blob is set to FREE

#define X_PADDING_LEFT   0
#define X_PADDING_REIGHT 0
#define Y_PADDING_TOP    0
#define Y_PADDING_BOTTOM 0
//#define MATRIX_RESOLUTION_X (NEW_FRAME - X_PADDING_LEFT - X_PADDING_REIGHT)
//#define MATRIX_RESOLUTION_Y (NEW_FRAME - Y_PADDING_TOP - Y_PADDING_BOTTOM)

#define X_MIN X_PADDING_LEFT                                  // Blob centroid X min value
#define X_MAX (NEW_FRAME - X_PADDING_LEFT - X_PADDING_REIGHT) // Blob centroid X max value
#define Y_MIN Y_PADDING_TOP                                   // Blob centroid Y min value
#define Y_MAX (NEW_FRAME - Y_PADDING_TOP- Y_PADDING_BOTTOM)   // Blob centroid Y max value

#define Z_MIN 0   // Blob centroid Z min value
#define Z_MAX 127 // Blob centroid Z max value

#define IIPi (float)(2 * PI)
#define IIIPiII (float)(3 * PI) / 2
#define PiII (float)(PI / 2)
#define LONG_HOLD 1500
#define LEVEL_TIMEOUT 3000
#define PENDING_MODE_TIMEOUT 4000

#define MIDI_THROTTLE_MS 10
#define MATRIX_MIDI_THROTTLE_MS 50 // 20Hz

#define VELOCITY_MIN_INTERVAL_MS 10    // Min time between velocity updates (ms)
#define VELOCITY_EMA_ALPHA       0.4f  // EMA smoothing factor: 0=frozen, 1=raw (no smoothing)
#define VELOCITY_ATTACK_MIN_MS   5     // Minimum guard before peak detection kicks in (ms)
#define VELOCITY_ATTACK_MAX_MS   80    // Hard deadline: force attack_done even without peak drop (ms)
#define VELOCITY_ATTACK_DROP     0.5f  // Peak-drop ratio: attack_done when |vz| < peak * ratio
#define VELOCITY_ATTACK_Z_MAX    4000  // Max expected |velocity.z| in units/s (tune to calibrate MIDI range)
#define VELOCITY_Z_DISPLAY_MAX   500   // Scale for SysEx Vz display (separate from attack — lower = more sensitive)
#define VELOCITY_XY_MAX          200   // Max expected velocity.xy in units/s for ROL sliders (tune to calibrate MIDI range)

// E256 MIDI I/O CHANNELS CONSTANTS [1:15]
#define MIDI_INPUT_CHANNEL 1

#define MIDI_CCS_CHANNEL 3
#define MIDI_MODES_CHANNEL 4
#define MIDI_VERBOSITY_CHANNEL 5
#define MIDI_ERROR_CHANNEL 6

// E256 MAPPING_LIB CONSTANTS
#define MAX_BLOBS 16 // [1:16] How many blobs can be tracked at the same time

#define MAX_SWITCHS 16
#define MAX_SWITCH_TOUCHS 2

#define MAX_SLIDERS 6
#define MAX_SLIDER_TOUCHS 2
#define MAX_SLIDER_STEPS 24

#define MAX_KNOBS 4
#define MAX_KNOB_TOUCHS 4

#define MAX_TOUCHPADS 2
#define MAX_TOUCHPAD_TOUCHS 5

#define MAX_POLYGONS 8
#define MAX_POLYGON_POINTS 64
#define MAX_POLYGON_TOUCHS 3

#define MAX_GRIDS 1
#define MAX_GRID_KEYS 256
#define MAX_GRID_TOUCHS 3

//#define MAX_CSLIDERS 2

typedef enum level_code_e {
  THRESHOLD, // E256-LEDs: | 1 | 1 |
  SIG_IN,    // E256-LEDs: | 1 | 0 |
  SIG_OUT,   // E256-LEDs: | 0 | 1 |
  LINE_OUT   // E256-LEDs: | 0 | 0 |
} level_code_t;

extern level_code_t e256_current_level;

// VERBOSITY MODES CONSTANTS
typedef enum mode_code_e {
  PENDING_MODE,    // Waiting for mode
  SYNC_MODE,       // Hand chake mode
  CALIBRATE_MODE,  // Proceed to the all matrix calibration
  MATRIX_RAW_MODE, // Send matrix analog sensor raw values (16x16) over USB using MIDI format
  MATRIX_INTERP_MODE, //
  MAPPING_MODE,    // 
  EDIT_MODE,       // Send all blobs values over USB_MIDI OUTPUT
  THROUGH_MODE,    // Forward mappings values to the MIDI_HARDWARE OUTPUT
  PLAY_MODE,       // Send mappings values to the USB_MIDI OUTPUT
  ALLOCATE_MODE,   //
  UPLOAD_MODE,     //
  APPLY_MODE,      //
  WRITE_MODE,      //
  LOAD_MODE,       //
  FETCH_MODE,      // Send mapping config file
  STANDALONE_MODE, // Send mappings values over MIDI hardware
  ERROR_MODE       // Unexpected behaviour
} mode_code_t;

extern mode_code_t e256_current_mode;

// VERBOSITY MODES CONSTANTS ACKNOWLEDGMENT
typedef enum verbosity_code_e{
  PENDING_MODE_DONE,
  SYNC_MODE_DONE,
  CALIBRATE_MODE_DONE,
  MATRIX_RAW_MODE_DONE,
  MATRIX_INTERP_MODE_DONE,
  MAPPING_MODE_DONE,
  EDIT_MODE_DONE,
  THROUGH_MODE_DONE,
  PLAY_MODE_DONE,
  ALLOCATE_MODE_DONE,
  ALLOCATE_DONE,
  UPLOAD_MODE_DONE,
  UPLOAD_DONE,
  CONFIG_APPLY_DONE,
  WRITE_MODE_DONE,
  LOAD_MODE_DONE,
  FETCH_MODE_DONE,
  STANDALONE_MODE_DONE,
  DONE_ACTION
} verbosity_code_t;

extern verbosity_code_t e256_verbosity_code;

typedef enum error_code_e {
  WAITING_FOR_CONFIG,
  CONNECTING_FLASH,
  FLASH_FULL,
  FILE_TO_BIG,
  NO_CONFIG_FILE,
  WHILE_OPEN_FLASH_FILE,
  USBMIDI_CONFIG_LOAD_FAILED,
  FLASH_CONFIG_LOAD_FAILED,
  FLASH_CONFIG_WRITE_FAILED,
  CONFIG_APPLY_FAILED,
  UNKNOWN_SYSEX,
  TOO_MANY_BLOBS
} error_code_t;

extern error_code_t e256_error_code;

typedef struct leds_s leds_t;
struct leds_s {
  bool D1;
  bool D2;
  bool update;
};

typedef struct e256_mode_s e256_mode_t;
struct e256_mode_s {
  leds_t leds;
  uint16_t timeOn;
  uint16_t timeOff;
  bool toggle;
};

typedef struct level_s level_t;
struct level_s {
  leds_t leds;
  uint8_t min_val;
  uint8_t max_val;
  uint8_t val;
  bool update;
};

typedef struct control_s control_t;
struct control_s {
  Encoder* encoder;
  e256_mode_t* modes;
  level_t* levels;
};

extern control_t e256_ctr;

extern uint8_t* flash_config_ptr;
extern size_t flash_config_size;

void blink(uint8_t iter, uint16_t delay_time);
void set_mode(mode_code_t mode);
void set_level(level_code_t level, uint8_t value);

void hardware_setup(void);
void update_controls(void);

bool load_flash_config(void);
bool mappings_apply_config(uint8_t* conf_ptr, size_t conf_size);

const char* get_mode_name(mode_code_t code);
const char* get_verbosity_name(verbosity_code_t code);
const char* get_level_name(level_code_t code);
const char* get_error_name(error_code_t code);

#endif /*__CONFIG_H__*/
