/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapp_switch.h"

struct mapp_switch_s;
typedef struct mapp_switch_s mapp_switch_t;
struct mapp_switch_s {
  common_t common;
  switch_t params;
  uint8_t active_blob_count;
  uint8_t touch_index;
};

static mapp_switch_t mapp_switches[MAX_SWITCHS];

llist_t llist_switch_pool;

bool mapping_switchs_alloc(uint8_t switchs_cnt) {
  if (switchs_cnt < MAX_SWITCHS) {
    llist_builder(&llist_switch_pool, &mapp_switches[0], switchs_cnt, sizeof(mapp_switches[0]));
    return true;
  }
  return false;
};

// Test if the blob is within the key limit
bool mapping_switch_is_blob_inside(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)mapping_ptr;
  
  if (blob_ptr->centroid.x > switch_ptr->params.rect.from.x &&
      blob_ptr->centroid.x < switch_ptr->params.rect.to.x &&
      blob_ptr->centroid.y > switch_ptr->params.rect.from.y &&
      blob_ptr->centroid.y < switch_ptr->params.rect.to.y) {
    return true;
  }
  return false;
};

bool mapping_switch_assign_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)mapping_ptr;

  if (switch_ptr->touch_index < switch_ptr->params.touchs) {
    blob_ptr->action.mapping_ptr = switch_ptr;
    blob_ptr->action.touch_ptr = &switch_ptr->params.touch[switch_ptr->touch_index];
    switch_ptr->touch_index++;
    switch_ptr->active_blob_count++;
    return true;
  }
  return false;
};

void mapping_switch_dispose_blob(common_t* mapping_ptr, blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)mapping_ptr;

  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.touch_ptr = NULL;
  switch_ptr->active_blob_count--;
  if (switch_ptr->active_blob_count == 0) {
    switch_ptr->touch_index = 0;
  }
};

void mapping_switch_start(blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)blob_ptr->action.mapping_ptr;
  touch_3d_t* touch_ptr = (touch_3d_t*)blob_ptr->action.touch_ptr;

  switch (switch_ptr->params.mode_z) {
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

void mapping_switch_continue(blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)blob_ptr->action.mapping_ptr;
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  if (switch_ptr->params.mode_z != NoteOn) {
    mapping_send_midi_msg(&touch_ptr->press, blob_ptr);
  }
};

void mapping_switch_stop(blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)blob_ptr->action.mapping_ptr;
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  switch (switch_ptr->params.mode_z) {
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

void mapping_switch_create(const JsonObject &config) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)llist_pop_front(&llist_switch_pool);

  switch_ptr->common.is_blob_inside_func_ptr = &mapping_switch_is_blob_inside;
  switch_ptr->common.blob_assign_func_ptr = &mapping_switch_assign_blob;
  switch_ptr->common.blob_dispose_func_ptr = &mapping_switch_dispose_blob;

  switch_ptr->common.start_func_ptr = &mapping_switch_start;
  switch_ptr->common.continue_func_ptr = &mapping_switch_continue;
  switch_ptr->common.stop_func_ptr = &mapping_switch_stop;
  
  switch_ptr->params.touchs = config["touchs"].as<uint8_t>();
  switch_ptr->params.rect.from.x = config["from"][0].as<float>();
  switch_ptr->params.rect.from.y = config["from"][1].as<float>();
  switch_ptr->params.rect.to.x = config["to"][0].as<float>();
  switch_ptr->params.rect.to.y = config["to"][1].as<float>();
  switch_ptr->params.mode_z = config["mode_z"].as<MidiType>();

  if (switch_ptr->params.touchs < MAX_SWITCH_TOUCHS) {
    midi_status_t status;
    for (uint8_t i = 0; i<switch_ptr->params.touchs; i++) {
    
    switch (switch_ptr->params.mode_z) {

      case NoteOn:
        midi_msg_status_unpack(config["msg"][i]["note"]["midi"]["status"].as<uint8_t>(), &status);
        switch_ptr->params.touch[i].note.type = NoteOn;
        switch_ptr->params.touch[i].note.data1 = config["msg"][i]["note"]["msg"]["data1"].as<uint8_t>();
        switch_ptr->params.touch[i].note.data2 = 0;
        switch_ptr->params.touch[i].note.channel = status.channel;
        break;

      case ControlChange:
        midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
        switch_ptr->params.touch[i].press.msg.type = status.type;
        switch_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
        switch_ptr->params.touch[i].press.msg.data2 = 0;
        switch_ptr->params.touch[i].press.msg.channel = status.channel;
        switch_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
        switch_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
        break;

      case AfterTouchPoly:
        midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
        switch_ptr->params.touch[i].press.msg.type = status.type;
        switch_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
        switch_ptr->params.touch[i].press.msg.data2 = 0;
        switch_ptr->params.touch[i].press.msg.channel = status.channel;
        switch_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
        switch_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
        
        midi_msg_status_unpack(config["msg"][i]["note"]["midi"]["status"].as<uint8_t>(), &status);
        switch_ptr->params.touch[i].note.type = status.type;
        switch_ptr->params.touch[i].note.data1 = config["msg"][i]["note"]["midi"]["data1"].as<uint8_t>();
        switch_ptr->params.touch[i].note.data2 = 0;
        switch_ptr->params.touch[i].note.channel = status.channel;
        break;
        
      default:
        break;
      }
    }
  }
  llist_push_back(&llist_mappings, switch_ptr);
};
