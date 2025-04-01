/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapp_slider.h"

typedef struct mapp_slider_s mapp_slider_t;
struct mapp_slider_s {
  common_t common;
  slider_t params;
  uint8_t active_blob_count;
  uint8_t touch_index;
};

static mapp_slider_t mapp_sliders[MAX_SLIDERS];

llist_t llist_sliders_pool;

void mapping_sliders_alloc(uint8_t sliders_cnt) {
  llist_builder(&llist_sliders_pool, &mapp_sliders[0], sliders_cnt, sizeof(mapp_sliders[0]));
};

bool mapping_slider_is_blob_inside(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;

  if (blob_ptr->centroid.x > slider_ptr->params.rect.from.x &&
      blob_ptr->centroid.x < slider_ptr->params.rect.to.x &&
      blob_ptr->centroid.y > slider_ptr->params.rect.from.y &&
      blob_ptr->centroid.y < slider_ptr->params.rect.to.y) {
    return true;
  }
  return false;
};

boolean mapping_slider_assign_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;
  
  if (slider_ptr->active_blob_count < slider_ptr->params.touchs) {
    //Serial.printf("\n_SLIDER_ASSIGN / TOUCH_INDEX: %d", slider_ptr->touch_index);
    //Serial.printf("\n_SLIDER_ASSIGN / BLOB_PTR: %p -> SLIDER_PTR: %p", blob_ptr, slider_ptr);
    blob_ptr->action.mapping_ptr = slider_ptr;
    blob_ptr->action.touch_ptr = &slider_ptr->params.touch[slider_ptr->touch_index];
    slider_ptr->touch_index++;
    slider_ptr->active_blob_count++;
    return true;
  }
  return false;
  //Serial.printf("\n_SLIDER_ASSIGN / ACTIVE_BLOB_COUNT: %d\tSLIDER_TOUCHS: %d",slider_ptr->active_blob_count, slider_ptr->params.touchs);
};

void mapping_slider_dispose_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;

  //Serial.printf("\nSLIDER_DISPOSE / BLOB_DISPOSE_MAPPING / BLOB: %p x MAPPING: %p", blob_ptr, mapping_ptr);

  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.touch_ptr = NULL;
  slider_ptr->active_blob_count--;
  if (slider_ptr->active_blob_count == 0) {
    slider_ptr->touch_index = 0;
  }

  //Serial.printf("\nSLIDER_DISPOSE / TOUCH_INDEX: %d", slider_ptr->touch_index);
  //Serial.printf("\nSLIDER_DISPOSE / ACTIVE_BLOB_COUNT: %d", slider_ptr->active_blob_count);
};

void mapping_slider_start(blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)blob_ptr->action.mapping_ptr;
  touch_2d_t* touch_ptr = (touch_2d_t*)blob_ptr->action.touch_ptr;

  //Serial.printf("\nSLIDER_START");

  switch (touch_ptr->press.midi.type) {
    case NoteOn:
      touch_ptr->last_midi_press = touch_ptr->pos.midi.data2;
      touch_ptr->press.midi.data2 = blob_ptr->centroid.z;
      midi_send_out(&touch_ptr->press.midi);
      break;
    case ControlChange:
      touch_ptr->last_midi_press = touch_ptr->pos.midi.data2;
      touch_ptr->press.midi.data2 = map(
        blob_ptr->centroid.z,
        Z_MIN,
        Z_MAX,
        touch_ptr->press.limit.min,
        touch_ptr->press.limit.max
      );
        midi_send_out(&touch_ptr->press.midi);
      break;
    case AfterTouchPoly:
      // Same as CC
      break;
    default:
      // Not handled in mapp_toucpad
      break;
  }

  switch (slider_ptr->params.dir) {
    case HORIZONTAL:
      touch_ptr->last_midi_pos = touch_ptr->pos.midi.data2;
      touch_ptr->pos.midi.data2 = round(map(
        blob_ptr->centroid.x,
        slider_ptr->params.rect.from.x,
        slider_ptr->params.rect.to.x,
        touch_ptr->pos.limit.min,
        touch_ptr->pos.limit.max)
      );
      if (touch_ptr->pos.midi.data2 != touch_ptr->last_midi_pos) {
        midi_send_out(&touch_ptr->pos.midi);
      }
      break;
    case VERTICAL:
      touch_ptr->last_midi_pos = touch_ptr->pos.midi.data2;
      touch_ptr->pos.midi.data2 = round(map(
        blob_ptr->centroid.y,
        slider_ptr->params.rect.from.y,
        slider_ptr->params.rect.to.y,
        touch_ptr->pos.limit.min,
        touch_ptr->pos.limit.max)
      );
      if (touch_ptr->pos.midi.data2 != touch_ptr->last_midi_pos) {
        midi_send_out(&touch_ptr->pos.midi);
      }
      break;
  };
  //Serial.printf("\nSLIDER_START / TOUCH_LAST_MIDI_PRESS: %d", touch_ptr->last_midi_press);
  //Serial.printf("\nSLIDER_START / TOUCH_LAST_MIDI_POS: %d", touch_ptr->last_midi_pos);
};

void mapping_slider_play(blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)blob_ptr->action.mapping_ptr;
  touch_2d_t* touch_ptr = (touch_2d_t*)blob_ptr->action.touch_ptr;

  switch (slider_ptr->params.dir) {
    case HORIZONTAL:
      touch_ptr->last_midi_pos = touch_ptr->pos.midi.data2;
      touch_ptr->pos.midi.data2 = round(map(
        blob_ptr->centroid.x,
        slider_ptr->params.rect.from.x,
        slider_ptr->params.rect.to.x,
        touch_ptr->pos.limit.min,
        touch_ptr->pos.limit.max)
      );
      if (touch_ptr->pos.midi.data2 != touch_ptr->last_midi_pos) {
        midi_send_out(&touch_ptr->pos.midi);
      }
      break;
    case VERTICAL:
      touch_ptr->last_midi_pos = touch_ptr->pos.midi.data2;
      touch_ptr->pos.midi.data2 = round(map(
        blob_ptr->centroid.y,
        slider_ptr->params.rect.from.y,
        slider_ptr->params.rect.to.y,
        touch_ptr->pos.limit.min,
        touch_ptr->pos.limit.max)
      );
      if (touch_ptr->pos.midi.data2 != touch_ptr->last_midi_pos) {
        midi_send_out(&touch_ptr->pos.midi);
      }
      break;
  };

  switch (touch_ptr->press.midi.type) {
    case ControlChange:
      touch_ptr->last_midi_press =  touch_ptr->press.midi.data2;
      touch_ptr->press.midi.data2 = map(
        blob_ptr->centroid.z,
        Z_MIN,
        Z_MAX,
        touch_ptr->press.limit.min,
        touch_ptr->press.limit.max
      );
      if (touch_ptr->press.midi.data2 != touch_ptr->last_midi_press) {
        midi_send_out(&touch_ptr->press.midi);
      }
      break;
    default:
      // Not handled
      break;
  }
};

void mapping_slider_stop(blob_t* blob_ptr) {
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  //touch_ptr->press.midi.type = NoteOff;
  touch_ptr->press.midi.data2 = 0;
  midi_send_out(&touch_ptr->press.midi);
};

void mapping_slider_create(const JsonObject &config) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)llist_pop_front(&llist_sliders_pool);

  slider_ptr->common.is_blob_inside_func_ptr = &mapping_slider_is_blob_inside;
  slider_ptr->common.blob_assign_func_ptr = &mapping_slider_assign_blob;
  slider_ptr->common.blob_dispose_func_ptr = &mapping_slider_dispose_blob;

  slider_ptr->common.start_func_ptr = &mapping_slider_start;
  slider_ptr->common.continue_func_ptr = &mapping_slider_play;
  slider_ptr->common.stop_func_ptr = &mapping_slider_stop;

  slider_ptr->params.touchs = config["touchs"].as<uint8_t>();

  slider_ptr->params.rect.from.x = config["from"][0].as<float>();
  slider_ptr->params.rect.from.y = config["from"][1].as<float>();
  slider_ptr->params.rect.to.x = config["to"][0].as<float>();
  slider_ptr->params.rect.to.y = config["to"][1].as<float>();

  midi_status_t status;
  for (uint8_t i = 0; i<slider_ptr->params.touchs; i++) {

    slider_ptr->params.touch[i].last_midi_pos = 0;
    midi_msg_status_unpack(config["msg"][i]["pos"]["midi"]["status"].as<uint8_t>(), &status);
    slider_ptr->params.touch[i].pos.midi.type = status.type;
    slider_ptr->params.touch[i].pos.midi.data1 = config["msg"][i]["pos"]["midi"]["data1"].as<uint8_t>();
    slider_ptr->params.touch[i].pos.midi.data2 = config["msg"][i]["pos"]["midi"]["data2"].as<uint8_t>();
    slider_ptr->params.touch[i].pos.midi.channel = status.channel;
    slider_ptr->params.touch[i].pos.limit.min = config["msg"][i]["pos"]["limit"]["min"].as<uint8_t>();
    slider_ptr->params.touch[i].pos.limit.max = config["msg"][i]["pos"]["limit"]["max"].as<uint8_t>();

    slider_ptr->params.touch[i].last_midi_press = 0;
    midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
    slider_ptr->params.touch[i].press.midi.type = status.type;
    slider_ptr->params.touch[i].press.midi.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
    slider_ptr->params.touch[i].press.midi.data2 = config["msg"][i]["press"]["midi"]["data2"].as<uint8_t>();
    slider_ptr->params.touch[i].press.midi.channel = status.channel;
    if (status.type == ControlChange || status.type == AfterTouchPoly) {
      slider_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
      slider_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
    }
    uint8_t size_x = slider_ptr->params.rect.to.x - slider_ptr->params.rect.from.x;
    uint8_t size_y = slider_ptr->params.rect.to.y - slider_ptr->params.rect.from.y;
    if (size_x < size_y) {
      slider_ptr->params.dir = VERTICAL;
    } else {
      slider_ptr->params.dir = HORIZONTAL;
    }
  }
  llist_push_back(&llist_mappings, slider_ptr);
};
