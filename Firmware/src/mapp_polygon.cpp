/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// Algorithm: http://alienryderflex.com/polygon

#include "mapp_polygon.h"

struct mapp_polygon_s;
typedef struct mapp_polygon_s mapp_polygon_t;
struct mapp_polygon_s {
  common_t common;
  polygon_t params;
  uint8_t active_blob_count;
  uint8_t touch_index;
  MidiType press;
  llist_t llist_active_midi_msg;
  uint8_t active_midi_msg_count;
};

static mapp_polygon_t mapp_polygons[MAX_POLYGONS];

llist_t llist_polygons_pool;

bool mapping_polygons_alloc(uint8_t polygons_cnt) {
  if (polygons_cnt <= MAX_POLYGONS) {
    llist_builder(&llist_polygons_pool, &mapp_polygons[0], polygons_cnt, sizeof(mapp_polygons[0]));
    return true;
  }
  return false;
};

// Test if the blob is within the polygon
bool mapping_polygon_is_blob_inside(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)mapping_ptr;
  int i, j = (polygon_ptr->params.point_cnt - 1);
  polygon_ptr->params.is_inside = false;
  for (i = 0; i < polygon_ptr->params.point_cnt; i++) {
    //float X1 = polygon_ptr->params.point[i].x;
    float Y1 = polygon_ptr->params.point[i].y;
    //float X2 = polygon_ptr->params.point[j].x;
    float Y2 = polygon_ptr->params.point[j].y;
    if ((Y1 < blob_ptr->centroid.y && Y2 >= blob_ptr->centroid.y) || (Y2 < blob_ptr->centroid.y && Y1 >= blob_ptr->centroid.y)) {
      // x ^= y; // equivalent to x = x ^ y;
      polygon_ptr->params.is_inside ^= ((blob_ptr->centroid.y * polygon_ptr->params.m[i] + polygon_ptr->params.c[i]) < blob_ptr->centroid.x);
    };
    j = i;
  };
  if (polygon_ptr->params.is_inside) {
    return true;
  }
  return false;
};

// blob == valeurs physiqyes captées
// touch == données du nieme blob
bool mapping_polygon_assign_blob(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)mapping_ptr;
  
  if (polygon_ptr->touch_index < polygon_ptr->params.touchs) {
    blob_ptr->action.mapping_ptr = polygon_ptr;
    blob_ptr->action.touch_ptr = &polygon_ptr->params.touch[polygon_ptr->touch_index];
    polygon_ptr->touch_index++;
    polygon_ptr->active_blob_count++;
    return true;
  }
  return false;
};

void mapping_polygon_dispose_blob(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)mapping_ptr;

  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.touch_ptr = NULL;
  polygon_ptr->active_blob_count--;
  if (polygon_ptr->active_blob_count == 0) {
    polygon_ptr->touch_index = 0;
  };
};

void mapping_polygon_start(blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)blob_ptr->action.mapping_ptr;
  touch_planar_t* touch_ptr = (touch_planar_t*)blob_ptr->action.touch_ptr;

  switch (polygon_ptr->params.press) {
    case NoteOn:
      mapping_send_midi_note_on(&touch_ptr->press, blob_ptr);
      break; 
    case ControlChange:
      mapping_send_midi_msg_press(&touch_ptr->press, blob_ptr);
      break;
    case AfterTouchPoly:
      mapping_send_midi_msg_press(&touch_ptr->press, blob_ptr);
      break;
    default:
      // Not handled in mapping_polygon
      break;
  }
};

void mapping_polygon_continue(blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)blob_ptr->action.mapping_ptr;
  touch_press_t* touch_ptr = (touch_press_t*)blob_ptr->action.touch_ptr;

  if (polygon_ptr->params.press != NoteOn) {
    mapping_send_midi_msg_press(&touch_ptr->press, blob_ptr);
  }
};

void mapping_polygon_stop(blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)blob_ptr->action.mapping_ptr;
  touch_press_t* touch_ptr = (touch_press_t*)blob_ptr->action.touch_ptr;

  switch (polygon_ptr->params.press) {
    case NoteOn:
      mapping_send_midi_note_off(&touch_ptr->press);
      break;
    case ControlChange:
      // N/A
      break;
    case AfterTouchPoly:
      // N/A
    break;
      default:
      // Not handled in mapping_polygon
      break;
  }
};

// IN PROGRESS!
bool mapping_polygon_hardware_midi_receive(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)mapping_ptr;
  if (midi_msg_ptr->channel == polygon_ptr->params.input_chan) { // FIXME
    return true;
  }
  return false;
};

// IN PROGRESS!
// Populates the MIDI polygon layout with the incoming MIDI notes/chord coming from a regular MIDI keyboard plugged in the e256 HARDWARE_MIDI_INPUT
void mapping_polygon_hardware_midi_update(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)mapping_ptr;
  llist_push_front(&polygon_ptr->llist_active_midi_msg, midi_msg_ptr);
  polygon_ptr->active_midi_msg_count++;
  //...
};

void mapping_polygon_hardware_midi_dispose(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)mapping_ptr;
  polygon_ptr->active_midi_msg_count--;
  if (polygon_ptr->active_midi_msg_count == 0) {  // Save/rescue all llist nodes
    midi_msg_t* midi_msg_ptr = NULL;
    while ((midi_msg_ptr = (midi_msg_t*)llist_pop_front(&polygon_ptr->llist_active_midi_msg)) != NULL) {
      llist_push_front(&llist_midi_nodes_pool, midi_msg_ptr);
    }
  }
};

void mapping_polygon_create(const JsonObject &config) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)llist_pop_front(&llist_polygons_pool);
  
  polygon_ptr->common.midi_hardware_receive_func_ptr = &mapping_polygon_hardware_midi_receive;  
  polygon_ptr->common.midi_hardware_update_func_ptr = &mapping_polygon_hardware_midi_update;  
  polygon_ptr->common.midi_hardware_dispose_func_ptr = &mapping_polygon_hardware_midi_dispose;

  polygon_ptr->common.is_blob_inside_func_ptr = &mapping_polygon_is_blob_inside;
  polygon_ptr->common.blob_assign_func_ptr = &mapping_polygon_assign_blob;
  polygon_ptr->common.blob_dispose_func_ptr = &mapping_polygon_dispose_blob;

  polygon_ptr->common.start_func_ptr = &mapping_polygon_start;
  polygon_ptr->common.continue_func_ptr = &mapping_polygon_continue;
  polygon_ptr->common.stop_func_ptr = &mapping_polygon_stop;

  polygon_ptr->params.touchs = config["touchs"].as<uint8_t>();
  polygon_ptr->params.press = config["press"].as<MidiType>();

  polygon_ptr->params.point_cnt = config["cnt"].as<uint8_t>();

  for (uint8_t i = 0; i < polygon_ptr->params.point_cnt; i++) {
    polygon_ptr->params.point[i].x = config["point"][i]["X"].as<float>();
    polygon_ptr->params.point[i].y = config["point"][i]["Y"].as<float>();
  };
  
  // For line equation y = mx + c, we pre-compute m and c for all edges of a given polygon
  float x1, x2, y1, y2;
  uint8_t v1, v2 = (polygon_ptr->params.point_cnt - 1);
  polygon_ptr->params.is_inside = false;
  for (v1 = 0; v1 < polygon_ptr->params.point_cnt; v1++) {
    x1 = polygon_ptr->params.point[v1].x;
    y1 = polygon_ptr->params.point[v1].y;
    x2 = polygon_ptr->params.point[v2].x;
    y2 = polygon_ptr->params.point[v2].y;
    if (y2 == y1) {
      polygon_ptr->params.c[v1] = x1;
      polygon_ptr->params.m[v1] = 0;
    }
    else {
      polygon_ptr->params.c[v1] = x1 - (y1 * x2) / (y2 - y1) + (y1 * x1) / (y2 - y1);
      polygon_ptr->params.m[v1] = (x2 - x1) / (y2 - y1);
    };
    v2 = v1;
  }
  llist_push_back(&llist_mappings, polygon_ptr);
};
