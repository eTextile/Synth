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

bool mapping_sliders_alloc(uint8_t sliders_cnt) {
  if (sliders_cnt < MAX_SLIDERS) {
    llist_builder(&llist_sliders_pool, &mapp_sliders[0], sliders_cnt, sizeof(mapp_sliders[0]));
    return true;
  }
  return false;
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

bool mapping_slider_assign_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;

  if (slider_ptr->touch_index < slider_ptr->params.touchs) {
    blob_ptr->action.mapping_ptr = slider_ptr;
    blob_ptr->action.touch_ptr = &slider_ptr->params.touch[slider_ptr->touch_index];    
    slider_ptr->touch_index++;
    slider_ptr->active_blob_count++;
    return true;
  }
  return false;
};

void mapping_slider_dispose_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;

  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.touch_ptr = NULL;
  slider_ptr->active_blob_count--;
  if (slider_ptr->active_blob_count == 0) {
    slider_ptr->touch_index = 0;
  }
};

void mapping_slider_start(blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)blob_ptr->action.mapping_ptr;
  touch_3d_t* touch_ptr = (touch_3d_t*)blob_ptr->action.touch_ptr;

  switch (slider_ptr->params.mode_z) {
    case NoteOn:
      mapping_send_note_on(&touch_ptr->note, blob_ptr);
      break;
    case ControlChange:
      mapping_send_midi_msg(&touch_ptr->press, blob_ptr);
      break;
    case AfterTouchPoly:
      // Send controlChange before NoteOn
      mapping_send_midi_msg(&touch_ptr->press, blob_ptr);
      mapping_send_note_on(&touch_ptr->note, blob_ptr);
      break;
    default:
      // Not handled in mapping_touchpad
      break;
  }
};

void mapping_slider_continue(blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)blob_ptr->action.mapping_ptr;
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  if (slider_ptr->params.mode_z != NoteOn) {
    mapping_send_midi_msg(&touch_ptr->press, blob_ptr);
  }
};

void mapping_slider_stop(blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)blob_ptr->action.mapping_ptr;
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  switch (slider_ptr->params.mode_z) {
    case NoteOn:
      mapping_send_note_off(&touch_ptr->note, blob_ptr);
      break;
    case ControlChange:
      // N/A
      break;
    case AfterTouchPoly:
      mapping_send_note_off(&touch_ptr->note, blob_ptr);
      break;
    default:
      // Not handled in mapp_switch
      break;
  }
};

void mapping_slider_create(const JsonObject &config) {
  
  mapp_slider_t* slider_ptr = (mapp_slider_t*)llist_pop_front(&llist_sliders_pool);

  slider_ptr->common.is_blob_inside_func_ptr = &mapping_slider_is_blob_inside;
  slider_ptr->common.blob_assign_func_ptr = &mapping_slider_assign_blob;
  slider_ptr->common.blob_dispose_func_ptr = &mapping_slider_dispose_blob;

  slider_ptr->common.start_func_ptr = &mapping_slider_start;
  slider_ptr->common.continue_func_ptr = &mapping_slider_continue;
  slider_ptr->common.stop_func_ptr = &mapping_slider_stop;

  slider_ptr->params.touchs = config["touchs"].as<uint8_t>();

  slider_ptr->params.rect.from.x = config["from"][0].as<float>();
  slider_ptr->params.rect.from.y = config["from"][1].as<float>();
  slider_ptr->params.rect.to.x = config["to"][0].as<float>();
  slider_ptr->params.rect.to.y = config["to"][1].as<float>();
  slider_ptr->params.mode_z = config["mode_z"].as<MidiType>();

  midi_status_t status;
  for (uint8_t i = 0; i<slider_ptr->params.touchs; i++) {

    midi_msg_status_unpack(config["msg"][i]["pos"]["midi"]["status"].as<uint8_t>(), &status);
    slider_ptr->params.touch[i].pos.msg.type = status.type;
    slider_ptr->params.touch[i].pos.msg.data1 = config["msg"][i]["pos"]["midi"]["data1"].as<uint8_t>();
    slider_ptr->params.touch[i].pos.msg.data2 = 0;
    slider_ptr->params.touch[i].pos.msg.channel = status.channel;
    slider_ptr->params.touch[i].pos.limit.min = config["msg"][i]["pos"]["limit"]["min"].as<uint8_t>();
    slider_ptr->params.touch[i].pos.limit.max = config["msg"][i]["pos"]["limit"]["max"].as<uint8_t>();

    switch (slider_ptr->params.mode_z) {

      case NoteOn:
        midi_msg_status_unpack(config["msg"][i]["note"]["midi"]["status"].as<uint8_t>(), &status);
        slider_ptr->params.touch[i].note.type = NoteOn;
        slider_ptr->params.touch[i].note.data1 = config["msg"][i]["note"]["midi"]["data1"].as<uint8_t>();
        slider_ptr->params.touch[i].note.data2 = 0;
        slider_ptr->params.touch[i].note.channel = status.channel;
        break;

      case ControlChange:
        midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
        slider_ptr->params.touch[i].press.msg.type = status.type;
        slider_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
        slider_ptr->params.touch[i].press.msg.data2 = 0;
        slider_ptr->params.touch[i].press.msg.channel = status.channel;
        slider_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
        slider_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
        break;

      case AfterTouchPoly:
        midi_msg_status_unpack(config["msg"][i]["note"]["midi"]["status"].as<uint8_t>(), &status);
        slider_ptr->params.touch[i].note.type = status.type;
        slider_ptr->params.touch[i].note.data1 = config["msg"][i]["note"]["midi"]["data1"].as<uint8_t>();
        slider_ptr->params.touch[i].note.data2 = 0;
        slider_ptr->params.touch[i].note.channel = status.channel;

        midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
        slider_ptr->params.touch[i].press.msg.type = status.type;
        slider_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
        slider_ptr->params.touch[i].press.msg.data2 = 0;
        slider_ptr->params.touch[i].press.msg.channel = status.channel;
        slider_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
        slider_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
        break;

      default:
        break;
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
