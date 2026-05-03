/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MAPPING_LIB_H__
#define __MAPPING_LIB_H__

#include <ArduinoJson.h>

#include "config.h"
#include "blob.h"
#include "llist.h"

#include "midi_bus.h"

typedef enum move_e {
  MOVE_LIN = 0,
  MOVE_LOG = 1,
  MOVE_ROL = 2
} move_t;

typedef enum populate_e {
  POPULATE_OFF       = 0,
  POPULATE_UP        = 1,
  POPULATE_DOWN      = 2,
  POPULATE_AS_PLAYED = 3,
  POPULATE_OCTAVE    = 4,
  POPULATE_PING_PONG = 5
} populate_t;

typedef struct point_s point_t;
struct point_s {
  float x;
  float y;
};

typedef struct rect_s rect_t;
struct rect_s {
  point_t from;
  point_t to;
};

typedef struct axis_s axis_t;
struct axis_s {
  midi_msg_t msg;
  midi_limit_t limit;
  uint8_t last_val;
  unsigned long int midi_time_stamp;
};

typedef struct touch_1d_s touch_1d_t;
struct touch_1d_s {
  axis_t press;
};

typedef struct touch_2d_s touch_2d_t;
struct touch_2d_s {
  axis_t press;
  axis_t pos;
};

typedef struct touch_3d_s touch_3d_t;
struct touch_3d_s {
  axis_t press;
  axis_t pos_x;
  axis_t pos_y;
};

extern llist_t llist_mappings;

typedef struct common_s common_t;

typedef bool midi_hardware_receive_func_t(void*, midi_msg_t*);
typedef void midi_hardware_update_func_t(void*, midi_msg_t*);
typedef void midi_hardware_dispose_func_t(void*, midi_msg_t*);

typedef bool is_blob_inside_func_t(void*, blob_t*);
typedef bool blob_assign_func_t(void*, blob_t*);
typedef void blob_dispose_func_t(void*, blob_t*);

typedef void start_func_t(blob_t*);
typedef void continue_func_t(blob_t*);
typedef void stop_func_t(blob_t*);

struct common_s {
  
  midi_hardware_receive_func_t* midi_hardware_receive_func_ptr;
  midi_hardware_update_func_t* midi_hardware_update_func_ptr;
  midi_hardware_dispose_func_t* midi_hardware_dispose_func_ptr;

  is_blob_inside_func_t* is_blob_inside_func_ptr;
  blob_assign_func_t* blob_assign_func_ptr;
  blob_dispose_func_t* blob_dispose_func_ptr;
  
  start_func_t* start_func_ptr;
  continue_func_t* continue_func_ptr;
  stop_func_t* stop_func_ptr;
};

void mapping_lib_update(void);

void mapping_send_midi_note_on(axis_t* axis_ptr, blob_t* blob_ptr);
void mapping_send_midi_note_on_xy(axis_t* axis_ptr, blob_t* blob_ptr);
void mapping_send_midi_note_off(axis_t* axis_ptr);

void mapping_send_midi_msg_pos_x(rect_t* bounding_box_ptr, axis_t* axis_ptr, blob_t* blob_ptr);
void mapping_send_midi_msg_pos_y(rect_t* bounding_box_ptr, axis_t* axis_ptr, blob_t* blob_ptr);
void mapping_send_midi_msg_press(axis_t* axis_ptr, blob_t* blob_ptr);

#endif /*__MAPPING_LIB_H__*/
