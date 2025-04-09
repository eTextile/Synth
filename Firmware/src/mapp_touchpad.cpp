/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapp_touchpad.h"

typedef struct mapp_touchpad_s mapp_touchpad_t;
struct mapp_touchpad_s {
  common_t common;
  touchpad_t params;
  uint8_t active_blob_count;
  uint8_t touch_index;
};

static mapp_touchpad_t mapp_touchpads[MAX_TOUCHPADS];

llist_t llist_touchpads_pool;

bool mapping_touchpads_alloc(uint8_t touchpads_cnt) {
  if (touchpads_cnt < MAX_TOUCHPADS) {
    llist_builder(&llist_touchpads_pool, &mapp_touchpads[0], touchpads_cnt, sizeof(mapp_touchpads[0]));
    return true;
  }
  return false;
};

bool mapping_touchpad_is_blob_inside(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)mapping_ptr;
  if (blob_ptr->centroid.x > touchpad_ptr->params.rect.from.x &&
        blob_ptr->centroid.x < touchpad_ptr->params.rect.to.x &&
        blob_ptr->centroid.y > touchpad_ptr->params.rect.from.y &&
        blob_ptr->centroid.y < touchpad_ptr->params.rect.to.y) {
    return true;
  }
  return false;
};

bool mapping_touchpad_assign_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)mapping_ptr;

  if (touchpad_ptr->touch_index < touchpad_ptr->params.touchs) {
    //Serial.printf("\n_TOUCHPAD_ASSIGN / BLOB_PTR: %p -> TOUCHPAD_PTR: %p", blob_ptr, touchpad_ptr);
    //Serial.printf("\n_TOUCHPAD_MAX_TOUCHS: %d", touchpad_ptr->params.touchs);
    //Serial.printf("\n_TOUCHPAD_ASSIGN / TOUCH_INDEX: %d", touchpad_ptr->touch_index);
    blob_ptr->action.mapping_ptr = touchpad_ptr;
    blob_ptr->action.touch_ptr = &touchpad_ptr->params.touch[touchpad_ptr->touch_index];

    touchpad_ptr->touch_index++;
    touchpad_ptr->active_blob_count++;
    //Serial.printf("\n_TOUCHPAD_ASSIGN / ACTIVE_BLOB_COUNT: %d",touchpad_ptr->active_blob_count); // DEBUG
    return true;
  }
  return false;
};

void mapping_touchpad_dispose_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)mapping_ptr;

  //Serial.printf("\n_TOUCHPAD_DISPOSE / BLOB_PTR: %p -> TOUCHPAD_PTR: %p", blob_ptr, mapping_ptr);
  
  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.touch_ptr = NULL;
  touchpad_ptr->active_blob_count--;
  if (touchpad_ptr->active_blob_count == 0) {
    touchpad_ptr->touch_index = 0;
  }
  //Serial.printf("\n_TOUCHPAD_DISPOSE / TOUCH_INDEX: %d", touchpad_ptr->touch_index);
  //Serial.printf("\n_TOUCHPAD_DISPOSE / ACTIVE_BLOB_COUNT: %d", touchpad_ptr->active_blob_count);
};

void mapping_touchpad_start(blob_t* blob_ptr) {
  touch_3d_t* touch_ptr = (touch_3d_t*)blob_ptr->action.touch_ptr;

  //Serial.printf("\n_TOUCHPAD_START");
  touch_ptr->last_midi_press = touch_ptr->press.midi.data2;
  switch (touch_ptr->press.midi.type) {
    case NoteOn:
      touch_ptr->press.midi.type = NoteOn;
      touch_ptr->press.midi.data2 = blob_ptr->centroid.z;
      midi_send_out(&touch_ptr->press.midi);
      break;
    case ControlChange:
      touch_ptr->press.midi.data2 = map(
        blob_ptr->centroid.z,
        Z_MIN,
        Z_MAX,
        touch_ptr->press.limit.min,
        touch_ptr->press.limit.max);
        midi_send_out(&touch_ptr->press.midi);
      break;
    case AfterTouchPoly:
      // Same as CC
      break;
    default:
      // Not handled in mapp_toucpad
      break;
  }
};

void mapping_touchpad_continue(blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)blob_ptr->action.mapping_ptr;
  touch_3d_t* touch_ptr = (touch_3d_t*)blob_ptr->action.touch_ptr;

  touch_ptr->last_midi_pos_x = touch_ptr->pos_x.midi.data2;
  touch_ptr->pos_x.midi.data2 = round(map(
    blob_ptr->centroid.x,
    touchpad_ptr->params.rect.from.x,
    touchpad_ptr->params.rect.to.x,
    touch_ptr->pos_x.limit.min,
    touch_ptr->pos_x.limit.max)
  );
  if (touch_ptr->pos_x.midi.data2 != touch_ptr->last_midi_pos_x) {
    midi_send_out(&touch_ptr->pos_x.midi);
  };

  touch_ptr->last_midi_pos_y = touch_ptr->pos_y.midi.data2;
  touch_ptr->pos_y.midi.data2 = round(map(
    blob_ptr->centroid.y,
    touchpad_ptr->params.rect.from.y,
    touchpad_ptr->params.rect.to.y,
    touch_ptr->pos_y.limit.min,
    touch_ptr->pos_y.limit.max)
  );
  if (touch_ptr->pos_y.midi.data2 != touch_ptr->last_midi_pos_y) {
    midi_send_out(&touch_ptr->pos_y.midi);
  };

  switch (touch_ptr->press.midi.type) {
    case NoteOn:
      // NA
      break;
    case ControlChange:
      touch_ptr->last_midi_press = touch_ptr->press.midi.data2;
      touch_ptr->press.midi.data2 = map(
        blob_ptr->centroid.z,
        Z_MIN,
        Z_MAX,
        touch_ptr->press.limit.min,
        touch_ptr->press.limit.max);
      if (touch_ptr->press.midi.data2 != touch_ptr->last_midi_press) {
        midi_send_out(&touch_ptr->press.midi);
      }
      break;
    case AfterTouchPoly:
      break;
    default:
      // Not handled in mapp_toucpad
      break;
  }
};

void mapping_touchpad_stop(blob_t* blob_ptr) {
  touch_3d_t* touch_ptr = (touch_3d_t*)blob_ptr->action.touch_ptr;
  //touch_ptr->press.midi.type = NoteOff;
  touch_ptr->press.midi.data2 = 0;
  midi_send_out(&touch_ptr->press.midi);
};

void mapping_touchpad_create(const JsonObject &config) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)llist_pop_front(&llist_touchpads_pool);

  touchpad_ptr->common.is_blob_inside_func_ptr = &mapping_touchpad_is_blob_inside;
  touchpad_ptr->common.blob_assign_func_ptr = &mapping_touchpad_assign_blob;
  touchpad_ptr->common.blob_dispose_func_ptr = &mapping_touchpad_dispose_blob;

  touchpad_ptr->common.start_func_ptr = &mapping_touchpad_start;
  touchpad_ptr->common.continue_func_ptr = &mapping_touchpad_continue;
  touchpad_ptr->common.stop_func_ptr = &mapping_touchpad_stop;

  touchpad_ptr->params.touchs = config["touchs"].as<uint8_t>();

  touchpad_ptr->params.rect.from.x = config["from"][0].as<float>();
  touchpad_ptr->params.rect.from.y = config["from"][1].as<float>();
  touchpad_ptr->params.rect.to.x = config["to"][0].as<float>();
  touchpad_ptr->params.rect.to.y = config["to"][1].as<float>();
  //touchpad_ptr->params.mode = config["mode_z"].as<uint8_t>();

  if (touchpad_ptr->params.touchs < MAX_TOUCHPAD_TOUCHS) {
    midi_status_t status;
    for (uint8_t i = 0; i<touchpad_ptr->params.touchs; i++){
      midi_msg_status_unpack(config["msg"][i]["pos_x"]["midi"]["status"].as<uint8_t>(), &status);
      touchpad_ptr->params.touch[i].pos_x.midi.type = status.type;
      touchpad_ptr->params.touch[i].pos_x.midi.data1 = config["msg"][i]["pos_x"]["midi"]["data1"].as<uint8_t>();
      touchpad_ptr->params.touch[i].pos_x.midi.data2 = config["msg"][i]["pos_x"]["midi"]["data2"].as<uint8_t>();
      touchpad_ptr->params.touch[i].pos_x.midi.channel = status.channel;
      if (status.type == ControlChange || status.type == AfterTouchPoly) {
        touchpad_ptr->params.touch[i].pos_x.limit.min = config["msg"][i]["pos_x"]["limit"]["min"].as<uint8_t>();
        touchpad_ptr->params.touch[i].pos_x.limit.max = config["msg"][i]["pos_x"]["limit"]["max"].as<uint8_t>();
      }
      midi_msg_status_unpack(config["msg"][i]["pos_y"]["midi"]["status"].as<uint8_t>(), &status);
      touchpad_ptr->params.touch[i].pos_y.midi.type = status.type;
      touchpad_ptr->params.touch[i].pos_y.midi.data1 = config["msg"][i]["pos_y"]["midi"]["data1"].as<uint8_t>();
      touchpad_ptr->params.touch[i].pos_y.midi.data2 = config["msg"][i]["pos_y"]["midi"]["data2"].as<uint8_t>();
      touchpad_ptr->params.touch[i].pos_y.midi.channel = status.channel;
      if (status.type == ControlChange || status.type == AfterTouchPoly) {
        touchpad_ptr->params.touch[i].pos_y.limit.min = config["msg"][i]["pos_y"]["limit"]["min"].as<uint8_t>();
        touchpad_ptr->params.touch[i].pos_y.limit.max = config["msg"][i]["pos_y"]["limit"]["max"].as<uint8_t>();
      }
      midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
      touchpad_ptr->params.touch[i].press.midi.type = status.type;
      touchpad_ptr->params.touch[i].press.midi.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
      touchpad_ptr->params.touch[i].press.midi.data2 = config["msg"][i]["press"]["midi"]["data2"].as<uint8_t>();
      touchpad_ptr->params.touch[i].press.midi.channel = status.channel;
      if (status.type == ControlChange || status.type == AfterTouchPoly) {
        touchpad_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
        touchpad_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
      }
    }
  }
  llist_push_back(&llist_mappings, touchpad_ptr);
};
