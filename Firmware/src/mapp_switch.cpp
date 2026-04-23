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
  llist_t llist_active_midi_msg;
  uint8_t active_midi_msg_count;
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
bool mapping_switch_is_blob_inside(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)mapping_ptr;
  
  if (blob_ptr->centroid.x > switch_ptr->params.rect.from.x &&
      blob_ptr->centroid.x < switch_ptr->params.rect.to.x &&
      blob_ptr->centroid.y > switch_ptr->params.rect.from.y &&
      blob_ptr->centroid.y < switch_ptr->params.rect.to.y) {
    return true;
  }
  return false;
};

bool mapping_switch_assign_blob(void* mapping_ptr, blob_t* blob_ptr) {
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

void mapping_switch_dispose_blob(void* mapping_ptr, blob_t* blob_ptr) {
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
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  switch (switch_ptr->params.press) {
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
      // Not handled in mapping_switch
      break;
  }
};

void mapping_switch_continue(blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)blob_ptr->action.mapping_ptr;
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  if (switch_ptr->params.press != NoteOn) {
    mapping_send_midi_msg_press(&touch_ptr->press, blob_ptr);
  }
};

void mapping_switch_stop(blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)blob_ptr->action.mapping_ptr;
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  switch (switch_ptr->params.press) {
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
      // Not handled in mapp_switch
      break;
  }
};

bool mapping_switch_midi_recive(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)mapping_ptr;
  if (midi_msg_ptr->channel == switch_ptr->params.recive_chan) {
    return true;
  }
  return false;
};

// IN PROGRESS!
// Populates the MIDI switch layout with the incomming MIDI notes/chord coming from a regular MIDI keyboard plugged in the e256 HARDWARE_MIDI_INPUT
void mapping_switch_midi_update(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)mapping_ptr;
  llist_push_front(&switch_ptr->llist_active_midi_msg, midi_msg_ptr);
  switch_ptr->active_midi_msg_count++;
  //...
};

void mapping_switch_midi_dispose(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)mapping_ptr;
  switch_ptr->active_midi_msg_count--;
  if (switch_ptr->active_midi_msg_count == 0) {  // Save/rescure all llist nodes
    midi_msg_t* midi_msg_ptr = NULL;
    while ((midi_msg_ptr = (midi_msg_t*)llist_pop_front(&switch_ptr->llist_active_midi_msg)) != NULL) {
      llist_push_front(&llist_midi_nodes_pool, midi_msg_ptr);
    }
  }
};

void mapping_switch_create(const JsonObject &config) {

  mapp_switch_t* switch_ptr = (mapp_switch_t*)llist_pop_front(&llist_switch_pool);

  switch_ptr->common.midi_recive_func_ptr = &mapping_switch_midi_recive;   // TESTING!
  switch_ptr->common.midi_update_func_ptr = &mapping_switch_midi_update;   // TESTING!
  switch_ptr->common.midi_dispose_func_ptr = &mapping_switch_midi_dispose; // TESTING!

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
  switch_ptr->params.press = config["press"].as<MidiType>();
  switch_ptr->params.recive_chan = config["recive"].as<uint8_t>();

  if (switch_ptr->params.touchs < MAX_SWITCH_TOUCHS) {
    
    for (uint8_t i = 0; i<switch_ptr->params.touchs; i++) {
      midi_status_t status;

      if (switch_ptr->params.press == NoteOn) {
        midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
        switch_ptr->params.touch[i].press.msg.type = status.type;
        switch_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
        //switch_ptr->params.touch[i].press.msg.data2 = 0;
        switch_ptr->params.touch[i].press.msg.channel = status.channel;
      }
      else {
        midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
        switch_ptr->params.touch[i].press.msg.type = status.type;
        switch_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
        //switch_ptr->params.touch[i].press.msg.data2 = 0;
        switch_ptr->params.touch[i].press.msg.channel = status.channel;
        switch_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
        switch_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
        break;
      }
    }
  llist_push_back(&llist_mappings, switch_ptr);
  }
};
