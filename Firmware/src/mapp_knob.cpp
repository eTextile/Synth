/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapp_knob.h"

typedef struct mapp_knob_s mapp_knob_t;
struct mapp_knob_s {
  common_t common;
  knob_t params;
  uint8_t active_blob_count;
  uint8_t touch_index;
  llist_t llist_active_midi_msg;
  uint8_t active_midi_msg_count;
};

static mapp_knob_t mapp_knobs[MAX_KNOBS];

llist_t llist_knobs_pool;

bool mapping_knobs_alloc(uint8_t knobs_cnt) {
  if (knobs_cnt < MAX_KNOBS) {
    llist_builder(&llist_knobs_pool, &mapp_knobs[0], knobs_cnt, sizeof(mapp_knobs[0]));
    return true;
  }
  return false;
};

bool mapping_knob_is_blob_inside(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)mapping_ptr;

  float x = blob_ptr->centroid.x - knob_ptr->params.center.x;
  float y = blob_ptr->centroid.y - knob_ptr->params.center.y;
  float radius = sqrt(x * x + y * y);
  if (radius < knob_ptr->params.radius) {
    return true;
  }
  return false;
};

bool mapping_knob_assign_blob(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)mapping_ptr;

  if (knob_ptr->touch_index < knob_ptr->params.touchs) {
    blob_ptr->action.mapping_ptr = knob_ptr;
    blob_ptr->action.touch_ptr = &knob_ptr->params.touch[knob_ptr->touch_index];
    knob_ptr->touch_index++;
    knob_ptr->active_blob_count++;
    return true;
  }
  return false;
};

void mapping_knob_dispose_blob(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)mapping_ptr;

  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.touch_ptr = NULL;
  knob_ptr->active_blob_count--;
  if (knob_ptr->active_blob_count == 0) {
    knob_ptr->touch_index = 0;
  }
};

void mapping_knob_start(blob_t* blob_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)blob_ptr->action.mapping_ptr;
  touch_3d_t* touch_ptr = (touch_3d_t*)blob_ptr->action.touch_ptr;

  switch (knob_ptr->params.press) {
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
      // Not handled in mapping_knob
      break;
  }
};

void mapping_knob_continue(blob_t* blob_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)blob_ptr->action.mapping_ptr;
  knob_touch_t* touch_ptr = (knob_touch_t*)blob_ptr->action.touch_ptr;
  
  //Serial.printf("\n_KNOB_CONTINUE: %s", get_type_name(knob_ptr->params.press));

  float x = blob_ptr->centroid.x - knob_ptr->params.center.x;
  float y = blob_ptr->centroid.y - knob_ptr->params.center.y;

  float theta = 0;
  // Rotation of Axes through an angle without shifting Origin
  float pos_x = x * cos(knob_ptr->params.offset) + y * sin(knob_ptr->params.offset);
  float pos_y = -x * sin(knob_ptr->params.offset) + y * cos(knob_ptr->params.offset);
  
  if (pos_x == 0 && 0 < pos_y) {
    theta = PiII;
  } else if (pos_x == 0 && pos_y < 0) {
    theta = IIIPiII;
  } else if (pos_x < 0) {
    theta = atanf(pos_y / pos_x) + PI;
  } else if (pos_y < 0) {
    theta = atanf(pos_y / pos_x) + IIPi;
  } else {
    theta = atanf(pos_y / pos_x);
  }

  touch_ptr->theta.last_val = touch_ptr->theta.msg.data2;

  touch_ptr->theta.msg.data2 = map(
    round(theta),
    0,
    127,
    touch_ptr->theta.limit.min,
    touch_ptr->theta.limit.max
  );

  if (touch_ptr->theta.msg.data2 != touch_ptr->theta.last_val) llist_push_front(&llist_midi_out, &touch_ptr->theta.msg); // Add a midi_msg to the llist_midi_out linked list

  touch_ptr->radius.last_val = touch_ptr->radius.msg.data2;
  touch_ptr->radius.msg.data2 = round(sqrt(x * x + y * y)); // Is it ok?
  
  if (touch_ptr->radius.msg.data2 != touch_ptr->radius.last_val) llist_push_front(&llist_midi_out, &touch_ptr->radius.msg);
  
  mapping_send_midi_msg_press(&touch_ptr->theta, blob_ptr);

};

void mapping_knob_stop(blob_t* blob_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)blob_ptr->action.mapping_ptr;
  touch_3d_t* touch_ptr = (touch_3d_t*)blob_ptr->action.touch_ptr;

  switch (knob_ptr->params.press) {
    case NoteOn:
      mapping_send_midi_note_off(&touch_ptr->press);
      break;
    case ControlChange:
      // N/A
      break;
    case AfterTouchPoly:
      //
      break;
    default:
      // Not handled in mapping_knob
      break;
  }
};

bool mapping_knob_hardware_midi_receive(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)mapping_ptr;
  if (midi_msg_ptr->channel == knob_ptr->params.input_chan) {
    return true;
  }
  return false;
};

// IN PROGRESS!
// Populates the MIDI grid layout with the incoming MIDI notes/chord coming from a regular MIDI keyboard plugged in the e256 HARDWARE_MIDI_INPUT
void mapping_knob_hardware_midi_update(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)mapping_ptr;
  llist_push_front(&knob_ptr->llist_active_midi_msg, midi_msg_ptr);
  knob_ptr->active_midi_msg_count++;
  //...
};

void mapping_knob_hardware_midi_dispose(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_knob_t* knob_ptr = (mapp_knob_t*)mapping_ptr;
  knob_ptr->active_midi_msg_count--;
  if (knob_ptr->active_midi_msg_count == 0) {  // Save/rescue all llist nodes
    midi_msg_t* midi_msg_ptr = NULL;
    while ((midi_msg_ptr = (midi_msg_t*)llist_pop_front(&knob_ptr->llist_active_midi_msg)) != NULL) {
      llist_push_front(&llist_midi_nodes_pool, midi_msg_ptr);
    }
  }
};

void mapping_knob_create(const JsonObject &config) {

  mapp_knob_t* knob_ptr = (mapp_knob_t*)llist_pop_front(&llist_knobs_pool);

  knob_ptr->common.midi_hardware_receive_func_ptr = &mapping_knob_hardware_midi_receive;   // TESTING!
  knob_ptr->common.midi_hardware_update_func_ptr = &mapping_knob_hardware_midi_update;   // TESTING!
  knob_ptr->common.midi_hardware_dispose_func_ptr = &mapping_knob_hardware_midi_dispose; // TESTING!

  knob_ptr->common.is_blob_inside_func_ptr = &mapping_knob_is_blob_inside;
  knob_ptr->common.blob_assign_func_ptr = &mapping_knob_assign_blob;
  knob_ptr->common.blob_dispose_func_ptr = &mapping_knob_dispose_blob;
  
  knob_ptr->common.start_func_ptr = &mapping_knob_start;
  knob_ptr->common.continue_func_ptr = &mapping_knob_continue;
  knob_ptr->common.stop_func_ptr = &mapping_knob_stop;
  
  knob_ptr->params.touchs = config["touchs"].as<uint8_t>();
  knob_ptr->params.rect.from.x = config["from"][0].as<float>();
  knob_ptr->params.rect.from.y = config["from"][1].as<float>();
  knob_ptr->params.rect.to.x = config["to"][0].as<float>();
  knob_ptr->params.rect.to.y = config["to"][1].as<float>();
  knob_ptr->params.offset = config["offset"].as<uint8_t>();
  knob_ptr->params.press = config["press"].as<MidiType>();
  knob_ptr->params.input_chan = config["input_chan"].as<uint8_t>();

  if (knob_ptr->params.touchs < MAX_KNOB_TOUCHS) {

    knob_ptr->params.radius = (knob_ptr->params.rect.to.x - knob_ptr->params.rect.from.x) / 2;
    knob_ptr->params.center.x = (knob_ptr->params.rect.from.x + knob_ptr->params.radius);
    knob_ptr->params.center.y = (knob_ptr->params.rect.from.y + knob_ptr->params.radius);
  
    midi_status_t status;
    for (uint8_t i = 0; i<knob_ptr->params.touchs; i++){
      midi_msg_status_unpack(config["msg"][i]["radius"]["midi"]["status"].as<uint8_t>(), &status);
      knob_ptr->params.touch[i].radius.msg.type = status.type;
      knob_ptr->params.touch[i].radius.msg.data1 = config["msg"][i]["radius"]["midi"]["data1"].as<uint8_t>();
      knob_ptr->params.touch[i].radius.msg.data2 = 0;
      knob_ptr->params.touch[i].radius.msg.channel = status.channel;
      knob_ptr->params.touch[i].radius.limit.min = config["msg"][i]["radius"]["limit"]["min"].as<uint8_t>();
      knob_ptr->params.touch[i].radius.limit.max = config["msg"][i]["radius"]["limit"]["max"].as<uint8_t>();
    
      midi_msg_status_unpack(config["msg"][i]["theta"]["midi"]["status"].as<uint8_t>(), &status);
      knob_ptr->params.touch[i].theta.msg.type = status.type;
      knob_ptr->params.touch[i].theta.msg.data1 = config["msg"][i]["theta"]["midi"]["data1"].as<uint8_t>();
      knob_ptr->params.touch[i].theta.msg.data2 = 0;
      knob_ptr->params.touch[i].theta.msg.channel = status.channel;
      knob_ptr->params.touch[i].theta.limit.min = config["msg"][i]["theta"]["limit"]["min"].as<uint8_t>();
      knob_ptr->params.touch[i].theta.limit.max = config["msg"][i]["theta"]["limit"]["max"].as<uint8_t>();

      switch (knob_ptr->params.press) {

        case NoteOn:
          midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
          knob_ptr->params.touch[i].press.msg.type = NoteOn;
          knob_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
          knob_ptr->params.touch[i].press.msg.data2 = 0;
          knob_ptr->params.touch[i].press.msg.channel = status.channel;
          break;

        case ControlChange:
          midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
          knob_ptr->params.touch[i].press.msg.type = status.type;
          knob_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
          knob_ptr->params.touch[i].press.msg.data2 = 0;
          knob_ptr->params.touch[i].press.msg.channel = status.channel;
          knob_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
          knob_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
          break;

        case AfterTouchPoly:
          midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
          knob_ptr->params.touch[i].press.msg.type = status.type;
          knob_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
          knob_ptr->params.touch[i].press.msg.data2 = 0;
          knob_ptr->params.touch[i].press.msg.channel = status.channel;
          knob_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
          knob_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
          break;

        default:
          break;
      }
    }
    llist_push_back(&llist_mappings, knob_ptr);
  }
};
