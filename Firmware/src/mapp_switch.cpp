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
    Serial.printf("\nSWITCH_ASSIGN / BLOB_COUNT: %d", switch_ptr->active_blob_count);
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
  Serial.printf("\nSWITCH_DISPOSE / BLOB_COUNT: %d", switch_ptr->active_blob_count);
};


void mapping_switch_start(blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)blob_ptr->action.mapping_ptr;
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  Serial.printf("\nSWITCH_START: %s", get_type_name(switch_ptr->params.mode_z));

  switch (switch_ptr->params.mode_z) {
    case NoteOn:
      //touch_ptr->note.midi.type = NoteOn; // Note nead
      touch_ptr->note.midi.data2 = blob_ptr->centroid.z;
      //touch_ptr->note.midi.data2 = 127;
      midi_send_out(&touch_ptr->note.midi);
      break;
    case NoteOff:
      touch_ptr->note.midi.type = NoteOn;
      touch_ptr->note.midi.data2 = blob_ptr->centroid.z;
      midi_send_out(&touch_ptr->note.midi);
      break;
    case ControlChange:
      touch_ptr->last_midi_press = touch_ptr->press.midi.data2;
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
      // Send controlChange before NoteOn
      touch_ptr->last_midi_press = touch_ptr->press.midi.data2;
      touch_ptr->press.midi.data2 = blob_ptr->centroid.z;
      midi_send_out(&touch_ptr->press.midi);

      touch_ptr->note.midi.type = NoteOn;
      touch_ptr->note.midi.data2 = blob_ptr->centroid.z;
      midi_send_out(&touch_ptr->note.midi);
      break;
    default:
      // Not handled in mapp_toucpad
      break;
  }
};

void mapping_switch_continue(blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)blob_ptr->action.mapping_ptr;
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  Serial.printf("\nSWITCH_CONTINUE: %s", get_type_name(switch_ptr->params.mode_z));

  if (switch_ptr->params.mode_z == ControlChange || switch_ptr->params.mode_z == AfterTouchPoly) {
    touch_ptr->last_midi_press = touch_ptr->press.midi.data2;
    touch_ptr->press.midi.data2 = blob_ptr->centroid.z;
    if (touch_ptr->press.midi.data2 != touch_ptr->last_midi_press) {
      midi_send_out(&touch_ptr->press.midi);
    }
  }
};

void mapping_switch_stop(blob_t* blob_ptr) {
  mapp_switch_t* switch_ptr = (mapp_switch_t*)blob_ptr->action.mapping_ptr;
  touch_1d_t* touch_ptr = (touch_1d_t*)blob_ptr->action.touch_ptr;

  Serial.printf("\nSWITCH_STOP %s", get_type_name(switch_ptr->params.mode_z));

  switch (switch_ptr->params.mode_z) {
    case NoteOn:
      touch_ptr->press.midi.type = NoteOff;
      touch_ptr->press.midi.data2 = 0;
      midi_send_out(&touch_ptr->press.midi);
      break;
    case NoteOff:
      touch_ptr->press.midi.data2 = 0;
      midi_send_out(&touch_ptr->press.midi);
      break;
    case ControlChange:
      touch_ptr->press.midi.data2 = 0; // USE or NOT_USE!?
      midi_send_out(&touch_ptr->press.midi);
      break;
    case AfterTouchPoly:
      touch_ptr->press.midi.type = NoteOff;
      touch_ptr->press.midi.data2 = 0;
      midi_send_out(&touch_ptr->press.midi);
      touch_ptr->press.midi.data2 = 0;
      midi_send_out(&touch_ptr->press.midi);
      break;
    default:
      // Not handled in mapp_toucpad
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
        switch_ptr->params.touch[i].note.midi.type = NoteOn;
        switch_ptr->params.touch[i].note.midi.data1 = config["msg"][i]["note"]["midi"]["data1"].as<uint8_t>();
        switch_ptr->params.touch[i].note.midi.data2 = config["msg"][i]["note"]["midi"]["data2"].as<uint8_t>();
        switch_ptr->params.touch[i].note.midi.channel = status.channel;
        break;
      case ControlChange:
        midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
        switch_ptr->params.touch[i].press.midi.type = status.type;
        switch_ptr->params.touch[i].press.midi.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
        switch_ptr->params.touch[i].press.midi.data2 = config["msg"][i]["press"]["midi"]["data2"].as<uint8_t>();
        switch_ptr->params.touch[i].press.midi.channel = status.channel;
        switch_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
        switch_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
        break;
      case AfterTouchPoly:
        midi_msg_status_unpack(config["msg"][i]["note"]["midi"]["status"].as<uint8_t>(), &status);
        switch_ptr->params.touch[i].note.midi.type = status.type;
        switch_ptr->params.touch[i].note.midi.data1 = config["msg"][i]["note"]["midi"]["data1"].as<uint8_t>();
        switch_ptr->params.touch[i].note.midi.data2 = config["msg"][i]["note"]["midi"]["data2"].as<uint8_t>();
        switch_ptr->params.touch[i].note.midi.channel = status.channel;

        midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
        switch_ptr->params.touch[i].press.midi.type = status.type;
        switch_ptr->params.touch[i].press.midi.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
        switch_ptr->params.touch[i].press.midi.data2 = config["msg"][i]["press"]["midi"]["data2"].as<uint8_t>();
        switch_ptr->params.touch[i].press.midi.channel = status.channel;
        switch_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
        switch_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
        break;
      default:
        break;
      }
    }
  }
  llist_push_back(&llist_mappings, switch_ptr);
};
