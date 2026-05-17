/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// Algorithm: http://alienryderflex.com/polygon

#include "mapp_polygon.h"

typedef struct mapp_polygon_s mapp_polygon_t;
struct mapp_polygon_s {
  common_t common;
  polygon_t params;
  uint8_t active_blob_count;
  uint8_t touch_index;
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
  }
};

void mapping_polygon_start(blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)blob_ptr->action.mapping_ptr;
  touch_polygon_t* touch_ptr = (touch_polygon_t*)blob_ptr->action.touch_ptr;

  if (polygon_ptr->params.press == NoteOn) {
    mapping_send_midi_note_on(&touch_ptr->press, blob_ptr);
  } else {
    mapping_send_midi_msg_press(&touch_ptr->press, blob_ptr);
  }
};

void mapping_polygon_continue(blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)blob_ptr->action.mapping_ptr;
  touch_polygon_t* touch_ptr = (touch_polygon_t*)blob_ptr->action.touch_ptr;

  if (polygon_ptr->params.press != NoteOn) {
    mapping_send_midi_msg_press(&touch_ptr->press, blob_ptr);
  }

  for (uint8_t vi = 0; vi < polygon_ptr->params.point_cnt; vi++) {
    if (!touch_ptr->source[vi].enabled) continue;
    float dx = blob_ptr->centroid.x - polygon_ptr->params.point[vi].x;
    float dy = blob_ptr->centroid.y - polygon_ptr->params.point[vi].y;
    float dist = sqrtf(dx * dx + dy * dy);
    uint8_t new_val = (uint8_t)constrain(
      (int)roundf(touch_ptr->source[vi].limit.max +
        (float)(touch_ptr->source[vi].limit.min - touch_ptr->source[vi].limit.max) *
        (dist / polygon_ptr->params.max_dist)),
      0, 127
    );
    if (new_val != touch_ptr->source[vi].last_val) {
      if ((millis() - touch_ptr->source[vi].midi_time_stamp) > MIDI_THROTTLE_MS) {
        touch_ptr->source[vi].msg.data2 = new_val;
        llist_push_front(&llist_midi_out, &touch_ptr->source[vi].msg);
        touch_ptr->source[vi].last_val = new_val;
        touch_ptr->source[vi].midi_time_stamp = millis();
      }
    }
  }
};

void mapping_polygon_stop(blob_t* blob_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)blob_ptr->action.mapping_ptr;
  touch_polygon_t* touch_ptr = (touch_polygon_t*)blob_ptr->action.touch_ptr;

  if (polygon_ptr->params.press == NoteOn) {
    mapping_send_midi_note_off(&touch_ptr->press);
  }
};

bool mapping_polygon_hardware_midi_receive(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_polygon_t* polygon_ptr = (mapp_polygon_t*)mapping_ptr;
  if (midi_msg_ptr->channel == polygon_ptr->params.input_chan) { // FIXME
    return true;
  }
  return false;
};

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
  
  polygon_ptr->common.hardware_midi_receive_func_ptr = &mapping_polygon_hardware_midi_receive;  
  polygon_ptr->common.hardware_midi_update_func_ptr = &mapping_polygon_hardware_midi_update;  
  polygon_ptr->common.hardware_midi_dispose_func_ptr = &mapping_polygon_hardware_midi_dispose;

  polygon_ptr->common.is_blob_inside_func_ptr = &mapping_polygon_is_blob_inside;
  polygon_ptr->common.blob_assign_func_ptr = &mapping_polygon_assign_blob;
  polygon_ptr->common.blob_dispose_func_ptr = &mapping_polygon_dispose_blob;

  polygon_ptr->common.start_func_ptr = &mapping_polygon_start;
  polygon_ptr->common.continue_func_ptr = &mapping_polygon_continue;
  polygon_ptr->common.stop_func_ptr = &mapping_polygon_stop;

  polygon_ptr->params.touchs = config["touchs"].as<uint8_t>();
  polygon_ptr->params.press = (MidiType)config["press"].as<uint8_t>();

  polygon_ptr->params.point_cnt = config["segments"].size();

  float min_x = 1e9f, max_x = -1e9f, min_y = 1e9f, max_y = -1e9f;
  for (uint8_t i = 0; i < polygon_ptr->params.point_cnt; i++) {
    polygon_ptr->params.point[i].x = config["segments"][i][0].as<float>();
    polygon_ptr->params.point[i].y = config["segments"][i][1].as<float>();
    if (polygon_ptr->params.point[i].x < min_x) min_x = polygon_ptr->params.point[i].x;
    if (polygon_ptr->params.point[i].x > max_x) max_x = polygon_ptr->params.point[i].x;
    if (polygon_ptr->params.point[i].y < min_y) min_y = polygon_ptr->params.point[i].y;
    if (polygon_ptr->params.point[i].y > max_y) max_y = polygon_ptr->params.point[i].y;
  };
  float bw = max_x - min_x;
  float bh = max_y - min_y;
  polygon_ptr->params.max_dist = sqrtf(bw * bw + bh * bh);
  if (polygon_ptr->params.max_dist < 1.0f) polygon_ptr->params.max_dist = 1.0f;

  // Pre-compute m and c for all edges (line equation y = mx + c)
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

  midi_status_t status;
  char key[16];
  for (uint8_t ti = 0; ti < polygon_ptr->params.touchs; ti++) {
    midi_msg_status_unpack(config["msg"][ti]["press"]["midi"]["status"].as<uint8_t>(), &status);
    polygon_ptr->params.touch[ti].press.msg.type = status.type;
    polygon_ptr->params.touch[ti].press.msg.data1 = config["msg"][ti]["press"]["midi"]["data1"].as<uint8_t>();
    polygon_ptr->params.touch[ti].press.msg.data2 = 0;
    polygon_ptr->params.touch[ti].press.msg.channel = status.channel;
    polygon_ptr->params.touch[ti].press.limit.min = config["msg"][ti]["press"]["limit"]["min"].as<uint8_t>();
    polygon_ptr->params.touch[ti].press.limit.max = config["msg"][ti]["press"]["limit"]["max"].as<uint8_t>();
    polygon_ptr->params.touch[ti].press.enabled = config["msg"][ti]["press"]["enabled"] | true;

    for (uint8_t vi = 0; vi < polygon_ptr->params.point_cnt; vi++) {
      snprintf(key, sizeof(key), "source_%u", vi);
      midi_msg_status_unpack(config["msg"][ti][key]["midi"]["status"].as<uint8_t>(), &status);
      polygon_ptr->params.touch[ti].source[vi].msg.type = ControlChange;
      polygon_ptr->params.touch[ti].source[vi].msg.data1 = config["msg"][ti][key]["midi"]["data1"].as<uint8_t>();
      polygon_ptr->params.touch[ti].source[vi].msg.data2 = 0;
      polygon_ptr->params.touch[ti].source[vi].msg.channel = status.channel;
      polygon_ptr->params.touch[ti].source[vi].limit.min = config["msg"][ti][key]["limit"]["min"].as<uint8_t>();
      polygon_ptr->params.touch[ti].source[vi].limit.max = config["msg"][ti][key]["limit"]["max"].as<uint8_t>();
      polygon_ptr->params.touch[ti].source[vi].enabled = config["msg"][ti][key]["enabled"] | true;
      polygon_ptr->params.touch[ti].source[vi].last_val = 255;
      polygon_ptr->params.touch[ti].source[vi].midi_time_stamp = 0;
    }
  }
  llist_push_back(&llist_mappings, polygon_ptr);
};
