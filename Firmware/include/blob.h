/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/src/omv/imlib/blob.c
    - This file is part of the OpenMV project.
    - Copyright (c) 2013- Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-Synthesizer project - https://synth.eTextile.org
    - Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __BLOB_H__
#define __BLOB_H__

#include "llist.h"

extern llist_t llist_blobs; // Blobs linked list
//extern llist_t llist_previous_blobs; // Exposed local declaration see blob.cpp

typedef enum blob_params_e {
  B_STATUS,        // [0]  STATUS_INDEX
  B_LAST_STATUS,   // [1]  LAST_STATUS_INDEX
  B_UID,           // [2]  UID_INDEX
  B_X_WHOLE,       // [3]  CENTROID_X_WHOLE_PART_INDEX
  B_X_FRAC,        // [4]  CENTROID_X_FRACTIONAL_PART_INDEX
  B_Y_WHOLE,       // [5]  CENTROID_Y_WHOLE_PART_INDEX
  B_Y_FRAC,        // [6]  CENTROID_Y_FRACTIONAL_PART_INDEX
  B_WIDTH,         // [7]  WIDTH_INDEX
  B_HEIGHT,        // [8]  HEIGHT_INDEX
  B_DEPTH,         // [9]  DEPTH_INDEX
  B_VELOCITY_XY,   // [10] VELOCITY_XY_INDEX
  B_VELOCITY_Z,    // [11] VELOCITY_Z_INDEX
  B_ATTACK_Z,      // [12] ATTACK_Z_INDEX
  B_ATTACK_DONE,   // [13] ATTACK_DONE_INDEX
  B_COUNT          // = 14
} blob_params_t;

typedef struct image_s image_t;
struct image_s {
  uint8_t* data_ptr;
  uint8_t num_cols;
  uint8_t num_rows;
};

#define COMPUTE_IMAGE_ROW_PTR(image_ptr, y) \
  ({ \
    __typeof__ (image_ptr) _image_ptr = (image_ptr); \
    __typeof__ (y) _y = (y); \
    ((uint8_t*)_image_ptr->data_ptr) + (_image_ptr->num_cols* _y); \
  })

#define IMAGE_GET_PIXEL_FAST(row_ptr, x) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
  })

#define IMAGE_SET_PIXEL_FAST(row_ptr, x, v) \
  ({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    __typeof__ (v) _v = (v); \
    _row_ptr[_x] = _v; \
  })

#define PIXEL_THRESHOLD(pixel, threshold) \
  ({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (threshold) _threshold = (threshold); \
    _pixel > _threshold; \
  })

#define MIN(a, b)({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define MAX(a, b)({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

typedef struct xylr_s xylr_t;
struct xylr_s {
  uint8_t x;
  uint8_t y;
  uint8_t l;
  uint8_t r;
  uint8_t t_l;
  uint8_t b_l;
};

typedef struct vertrice_s vertrice_t;
struct vertrice_s {
  float x;
  float y;
  uint8_t z;
};

typedef struct box_s box_t;
struct box_s {
  uint8_t w; // TODO: make it as float // use from
  uint8_t h; // TODO: make it as float // use to
};

typedef struct velocity_s velocity_t;
struct velocity_s {
  unsigned long time_stamp;    // timestamp of last z velocity update (10ms interval)
  unsigned long xy_time_stamp; // timestamp of last xy velocity update (1ms interval)
  unsigned long born_at;       // timestamp when blob was first detected (status == NEW)
  float xy_last_x;             // centroid.x at last xy velocity update
  float xy_last_y;             // centroid.y at last xy velocity update
  float xy;                    // smoothed XY velocity (units/s)
  float z;                     // smoothed Z velocity (units/s, signed: >0 pressing, <0 releasing)
  float attack_z;              // peak |velocity.z| captured during the attack window
  bool  attack_done;           // true once peak drop detected or VELOCITY_ATTACK_MAX_MS elapsed
};

typedef enum status_code_e {
  FREE,
  NEW,
  PRESENT,
  MISSING,
  RELEASED
} status_code_t;

typedef struct blob_action_s blob_action_t;
struct blob_action_s {
  void* mapping_ptr;
  void* touch_ptr;
  bool  note_on_z_pending;  // true = NoteOn deferred until z-attack peak captured
  bool  note_on_xy_pending; // true = NoteOn deferred until first xy velocity sample (ROL sliders)
};

typedef struct blob_s blob_t;
struct blob_s {
  uint8_t UID;
  status_code_t status;
  status_code_t last_status;
  uint32_t life_time_stamp;
  box_t box;
  uint16_t pixels;
  vertrice_t centroid;
  vertrice_t last_centroid; 
  velocity_t velocity;
  blob_action_t action;
};

void blob_setup(void);
void matrix_find_blobs(void);

bool is_blob_existing(blob_t* blob_ptr, blob_t* new_blob_ptr);
const char* get_blob_status_name(status_code_t code);

#endif /*__BLOB_H__*/
