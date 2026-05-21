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
  llist_t llist_active_midi_msg;
  uint8_t active_midi_msg_count;
  midi_msg_t chord_notes[MAX_TOUCHPAD_TOUCHS][MAX_CHORD_NOTES];
};

static mapp_touchpad_t mapp_touchpads[MAX_TOUCHPADS];

llist_t llist_touchpads_pool;

bool mapping_touchpads_alloc(uint8_t touchpads_cnt) {
  if (touchpads_cnt <= MAX_TOUCHPADS) {
    llist_builder(&llist_touchpads_pool, &mapp_touchpads[0], touchpads_cnt, sizeof(mapp_touchpads[0]));
    return true;
  }
  return false;
};

bool mapping_touchpad_is_blob_inside(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)mapping_ptr;
  if (blob_ptr->centroid.x > touchpad_ptr->params.rect.from.x &&
      blob_ptr->centroid.x < touchpad_ptr->params.rect.to.x &&
      blob_ptr->centroid.y > touchpad_ptr->params.rect.from.y &&
      blob_ptr->centroid.y < touchpad_ptr->params.rect.to.y) {
    return true;
  }
  return false;
};

bool mapping_touchpad_assign_blob(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)mapping_ptr;

  if (touchpad_ptr->touch_index < touchpad_ptr->params.touchs) {
    blob_ptr->action.mapping_ptr = touchpad_ptr;
    blob_ptr->action.touch_ptr = &touchpad_ptr->params.touch[touchpad_ptr->touch_index];
    touchpad_ptr->touch_index++;
    touchpad_ptr->active_blob_count++;
    return true;
  }
  return false;
};

void mapping_touchpad_dispose_blob(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)mapping_ptr;

  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.touch_ptr = NULL;
  touchpad_ptr->active_blob_count--;
  if (touchpad_ptr->active_blob_count == 0) {
    touchpad_ptr->touch_index = 0;
  }
};

void mapping_touchpad_start(blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)blob_ptr->action.mapping_ptr;
  touch_planar_t* touch_ptr = (touch_planar_t*)blob_ptr->action.touch_ptr;

  uint8_t ti = (uint8_t)(touch_ptr - &touchpad_ptr->params.touch[0]);
  switch (touchpad_ptr->params.press) {
    case NoteOn:
      mapping_send_midi_note_on(&touch_ptr->press, blob_ptr);
      break;
    case MIDI_TYPE_CHORD:
      midi_send_chord_on(touchpad_ptr->chord_notes[ti], &touchpad_ptr->params.chord[ti],
                         touchpad_ptr->params.input_chan,
                         (uint8_t)map(blob_ptr->centroid.z, Z_MIN, Z_MAX, 1, 127));
      break;
    default:
      mapping_send_midi_msg_press(&touch_ptr->press, blob_ptr);
      break;
  }
};

void mapping_touchpad_continue(blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)blob_ptr->action.mapping_ptr;
  touch_planar_t* touch_ptr = (touch_planar_t*)blob_ptr->action.touch_ptr;

  mapping_send_midi_msg_pos_x(&touchpad_ptr->params.rect, &touch_ptr->pos_x, blob_ptr);
  mapping_send_midi_msg_pos_y(&touchpad_ptr->params.rect, &touch_ptr->pos_y, blob_ptr);
  
  if (touchpad_ptr->params.press != NoteOn) {
    mapping_send_midi_msg_press(&touch_ptr->press, blob_ptr);
  }
};

void mapping_touchpad_stop(blob_t* blob_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)blob_ptr->action.mapping_ptr;
  touch_planar_t* touch_ptr = (touch_planar_t*)blob_ptr->action.touch_ptr;

  uint8_t ti = (uint8_t)(touch_ptr - &touchpad_ptr->params.touch[0]);
  switch (touchpad_ptr->params.press) {
    case NoteOn:
      mapping_send_midi_note_off(&touch_ptr->press);
      break;
    case MIDI_TYPE_CHORD:
      midi_send_chord_off(touchpad_ptr->chord_notes[ti], touchpad_ptr->params.chord[ti].type);
      break;
    default:
      break;
  }
};

bool mapping_touchpad_hardware_midi_receive(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)mapping_ptr;
  if (midi_msg_ptr->channel == touchpad_ptr->params.input_chan) {
    return true;
  }
  return false;
};

void mapping_touchpad_hardware_midi_update(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)mapping_ptr;
  llist_push_front(&touchpad_ptr->llist_active_midi_msg, midi_msg_ptr);
  touchpad_ptr->active_midi_msg_count++;
  //...
};

void mapping_touchpad_hardware_midi_dispose(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)mapping_ptr;
  touchpad_ptr->active_midi_msg_count--;
  if (touchpad_ptr->active_midi_msg_count == 0) {  // Save/rescue all llist nodes
    midi_msg_t* midi_msg_ptr = NULL;
    while ((midi_msg_ptr = (midi_msg_t*)llist_pop_front(&touchpad_ptr->llist_active_midi_msg)) != NULL) {
      llist_push_front(&llist_midi_nodes_pool, midi_msg_ptr);
    }
  }
};

void mapping_touchpad_create(const JsonObject &config) {

  mapp_touchpad_t* touchpad_ptr = (mapp_touchpad_t*)llist_pop_front(&llist_touchpads_pool);

  touchpad_ptr->common.hardware_midi_receive_func_ptr = &mapping_touchpad_hardware_midi_receive;  
  touchpad_ptr->common.hardware_midi_update_func_ptr = &mapping_touchpad_hardware_midi_update;  
  touchpad_ptr->common.hardware_midi_dispose_func_ptr = &mapping_touchpad_hardware_midi_dispose;

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
  touchpad_ptr->params.press = (MidiType)config["press"].as<uint8_t>();
  touchpad_ptr->params.input_chan = config["input_chan"].as<uint8_t>();

  if (touchpad_ptr->params.touchs <= MAX_TOUCHPAD_TOUCHS) {
    
    midi_status_t status;
    for (uint8_t i = 0; i<touchpad_ptr->params.touchs; i++) {

      midi_msg_status_unpack(config["msg"][i]["pos_x"]["midi"]["status"].as<uint8_t>(), &status);
      // if status.type != Undefined_F4
      touchpad_ptr->params.touch[i].pos_x.msg.type = ControlChange;
      touchpad_ptr->params.touch[i].pos_x.msg.data1 = config["msg"][i]["pos_x"]["midi"]["data1"].as<uint8_t>();
      touchpad_ptr->params.touch[i].pos_x.msg.data2 = 0;
      touchpad_ptr->params.touch[i].pos_x.msg.channel = status.channel;
      touchpad_ptr->params.touch[i].pos_x.limit.min = config["msg"][i]["pos_x"]["limit"]["min"].as<uint8_t>();
      touchpad_ptr->params.touch[i].pos_x.limit.max = config["msg"][i]["pos_x"]["limit"]["max"].as<uint8_t>();
      touchpad_ptr->params.touch[i].pos_x.enabled = config["msg"][i]["pos_x"]["enabled"] | true;

      midi_msg_status_unpack(config["msg"][i]["pos_y"]["midi"]["status"].as<uint8_t>(), &status);
      // if status.type != Undefined_F4
      touchpad_ptr->params.touch[i].pos_y.msg.type = ControlChange;
      touchpad_ptr->params.touch[i].pos_y.msg.data1 = config["msg"][i]["pos_y"]["midi"]["data1"].as<uint8_t>();
      touchpad_ptr->params.touch[i].pos_y.msg.data2 = 0;
      touchpad_ptr->params.touch[i].pos_y.msg.channel = status.channel;
      touchpad_ptr->params.touch[i].pos_y.limit.min = config["msg"][i]["pos_y"]["limit"]["min"].as<uint8_t>();
      touchpad_ptr->params.touch[i].pos_y.limit.max = config["msg"][i]["pos_y"]["limit"]["max"].as<uint8_t>();
      touchpad_ptr->params.touch[i].pos_y.enabled = config["msg"][i]["pos_y"]["enabled"] | true;

      switch (touchpad_ptr->params.press) {
        case NoteOn:
          midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
          touchpad_ptr->params.touch[i].press.msg.type = NoteOn;
          touchpad_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
          touchpad_ptr->params.touch[i].press.msg.data2 = 0;
          touchpad_ptr->params.touch[i].press.msg.channel = status.channel;
          touchpad_ptr->params.touch[i].press.enabled = config["msg"][i]["press"]["enabled"] | true;
          break;
        case ControlChange:
        case AfterTouchPoly:
          midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
          touchpad_ptr->params.touch[i].press.msg.type = status.type;
          touchpad_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
          touchpad_ptr->params.touch[i].press.msg.data2 = 0;
          touchpad_ptr->params.touch[i].press.msg.channel = status.channel;
          touchpad_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
          touchpad_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
          touchpad_ptr->params.touch[i].press.enabled = config["msg"][i]["press"]["enabled"] | true;
          break;
        case MIDI_TYPE_CHORD:
          touchpad_ptr->params.chord[i].type = config["msg"][i]["press"]["chord"].as<uint8_t>();
          touchpad_ptr->params.chord[i].note = config["msg"][i]["press"]["note"].as<uint8_t>();
          break;
        default:
          // None (0xFF) — no press output
          break;
      }
    }
    llist_push_back(&llist_mappings, touchpad_ptr);
  }
};
