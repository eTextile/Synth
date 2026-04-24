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
  llist_t llist_active_midi_msg;
  uint8_t active_midi_msg_count;
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

bool mapping_slider_is_blob_inside(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;

  if (blob_ptr->centroid.x > slider_ptr->params.rect.from.x &&
      blob_ptr->centroid.x < slider_ptr->params.rect.to.x &&
      blob_ptr->centroid.y > slider_ptr->params.rect.from.y &&
      blob_ptr->centroid.y < slider_ptr->params.rect.to.y) {
    return true;
  }
  return false;
};

bool mapping_slider_assign_blob(void* mapping_ptr, blob_t* blob_ptr) {
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

void mapping_slider_dispose_blob(void* mapping_ptr, blob_t* blob_ptr) {
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
  touch_2d_t* touch_ptr = (touch_2d_t*)blob_ptr->action.touch_ptr;

  if (slider_ptr->params.move == MOVE_ROL && slider_ptr->params.steps > 0) {
    uint8_t step_idx;
    if (slider_ptr->params.dir == VERTICAL) {
      step_idx = (uint8_t)map(blob_ptr->centroid.y,
        slider_ptr->params.rect.from.y, slider_ptr->params.rect.to.y,
        0, slider_ptr->params.steps - 1);
    } else {
      step_idx = (uint8_t)map(blob_ptr->centroid.x,
        slider_ptr->params.rect.from.x, slider_ptr->params.rect.to.x,
        0, slider_ptr->params.steps - 1);
    }
    step_idx = constrain(step_idx, 0, slider_ptr->params.steps - 1);
    touch_ptr->press.msg.data1 = slider_ptr->params.step_note[step_idx];
  }

  switch (slider_ptr->params.press) {
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
      // Not handled in mapping_slider
      break;
  }
};

void mapping_slider_continue(blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)blob_ptr->action.mapping_ptr;
  touch_2d_t* touch_ptr = (touch_2d_t*)blob_ptr->action.touch_ptr;

  if (slider_ptr->params.move == MOVE_ROL && slider_ptr->params.steps > 0) {
    uint8_t step_idx;
    if (slider_ptr->params.dir == VERTICAL) {
      step_idx = (uint8_t)map(blob_ptr->centroid.y,
        slider_ptr->params.rect.from.y, slider_ptr->params.rect.to.y,
        0, slider_ptr->params.steps - 1);
    } else {
      step_idx = (uint8_t)map(blob_ptr->centroid.x,
        slider_ptr->params.rect.from.x, slider_ptr->params.rect.to.x,
        0, slider_ptr->params.steps - 1);
    }
    step_idx = constrain(step_idx, 0, slider_ptr->params.steps - 1);
    uint8_t new_note = slider_ptr->params.step_note[step_idx];
    if (new_note != touch_ptr->press.msg.data1) {
      mapping_send_midi_note_off(&touch_ptr->press);
      touch_ptr->press.msg.data1 = new_note;
      mapping_send_midi_note_on(&touch_ptr->press, blob_ptr);
    }
  }
  else {
    if (slider_ptr->params.dir == HORIZONTAL) {
      mapping_send_midi_msg_pos_x(&slider_ptr->params.rect, &touch_ptr->pos, blob_ptr);
    }
    else {
      mapping_send_midi_msg_pos_y(&slider_ptr->params.rect, &touch_ptr->pos, blob_ptr);
    }
    if (slider_ptr->params.press != NoteOn) {
      mapping_send_midi_msg_press(&touch_ptr->press, blob_ptr);
    }
  }
};

void mapping_slider_stop(blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)blob_ptr->action.mapping_ptr;
  touch_2d_t* touch_ptr = (touch_2d_t*)blob_ptr->action.touch_ptr;
  if (slider_ptr->params.press == NoteOn) {
    mapping_send_midi_note_off(&touch_ptr->press);
  }
};

bool mapping_slider_midi_receive(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;
  if (midi_msg_ptr->channel == slider_ptr->params.receive_chan) {
    return true;
  }
  return false;
};

// Populates the MIDI slider layout with the incoming MIDI notes/chord coming from a regular MIDI keyboard plugged in the e256 HARDWARE_MIDI_INPUT
void mapping_slider_midi_update(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;
  llist_push_front(&slider_ptr->llist_active_midi_msg, midi_msg_ptr);
  slider_ptr->active_midi_msg_count++;

  uint8_t steps = slider_ptr->params.steps;
  if (steps == 0) return;

  switch (slider_ptr->params.populate) {

    case POPULATE_OFF:
      break;

    case POPULATE_AS_PLAYED: {
      uint8_t zone = (slider_ptr->active_midi_msg_count - 1) % steps;
      slider_ptr->params.step_note[zone] = midi_msg_ptr->data1;
      break;
    }

    case POPULATE_UP: {
      uint8_t notes[MAX_SLIDER_STEPS] = {0};
      uint8_t count = 0;
      for (lnode_t* node = ITERATOR_START_FROM_HEAD(&slider_ptr->llist_active_midi_msg);
           node != NULL && count < steps;
           node = ITERATOR_NEXT(node), count++) {
        notes[count] = ((midi_msg_t*)ITERATOR_DATA(node))->data1;
      }
      for (uint8_t a = 0; a < count - 1; a++) {
        for (uint8_t b = 0; b < count - 1 - a; b++) {
          if (notes[b] > notes[b + 1]) { uint8_t t = notes[b]; notes[b] = notes[b+1]; notes[b+1] = t; }
        }
      }
      for (uint8_t i = 0; i < count; i++) {
        slider_ptr->params.step_note[i] = notes[i];
      }
      break;
    }

    case POPULATE_DOWN: {
      uint8_t notes[MAX_SLIDER_STEPS] = {0};
      uint8_t count = 0;
      for (lnode_t* node = ITERATOR_START_FROM_HEAD(&slider_ptr->llist_active_midi_msg);
           node != NULL && count < steps;
           node = ITERATOR_NEXT(node), count++) {
        notes[count] = ((midi_msg_t*)ITERATOR_DATA(node))->data1;
      }
      for (uint8_t a = 0; a < count - 1; a++) {
        for (uint8_t b = 0; b < count - 1 - a; b++) {
          if (notes[b] < notes[b + 1]) { uint8_t t = notes[b]; notes[b] = notes[b+1]; notes[b+1] = t; }
        }
      }
      for (uint8_t i = 0; i < count; i++) {
        slider_ptr->params.step_note[i] = notes[i];
      }
      break;
    }

    case POPULATE_OCTAVE: {
      uint8_t base = midi_msg_ptr->data1;
      for (uint8_t i = 0; i < steps; i++) {
        slider_ptr->params.step_note[i] = base + i;
      }
      break;
    }

    case POPULATE_PING_PONG: {
      if (steps > 1) {
        uint8_t period = 2 * (steps - 1);
        uint8_t pos = (slider_ptr->active_midi_msg_count - 1) % period;
        uint8_t zone = (pos < steps) ? pos : period - pos;
        slider_ptr->params.step_note[zone] = midi_msg_ptr->data1;
      }
      break;
    }
  }
};

void mapping_slider_midi_dispose(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;
  slider_ptr->active_midi_msg_count--;
  if (slider_ptr->active_midi_msg_count == 0) {  // Save/rescue all llist nodes
    midi_msg_t* midi_msg_ptr = NULL;
    while ((midi_msg_ptr = (midi_msg_t*)llist_pop_front(&slider_ptr->llist_active_midi_msg)) != NULL) {
      llist_push_front(&llist_midi_nodes_pool, midi_msg_ptr);
    }
  }
};

void mapping_slider_create(const JsonObject &config) {
  
  mapp_slider_t* slider_ptr = (mapp_slider_t*)llist_pop_front(&llist_sliders_pool);

  slider_ptr->common.midi_receive_func_ptr = &mapping_slider_midi_receive;   // TESTING!
  slider_ptr->common.midi_update_func_ptr = &mapping_slider_midi_update;   // TESTING!
  slider_ptr->common.midi_dispose_func_ptr = &mapping_slider_midi_dispose; // TESTING!

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
  slider_ptr->params.press = config["press"].as<MidiType>();
  slider_ptr->params.move = config["move"].as<move_t>();
  slider_ptr->params.populate = config["populate"].as<populate_t>();
  slider_ptr->params.steps = config["steps"].as<uint8_t>();
  slider_ptr->params.receive_chan = config["receive"].as<uint8_t>();
  for (uint8_t i = 0; i < MAX_SLIDER_STEPS; i++) {
    slider_ptr->params.step_note[i] = 60 + i;
  }
  if (slider_ptr->params.touchs < MAX_SLIDER_TOUCHS) {

    midi_status_t status;
    for (uint8_t i = 0; i<slider_ptr->params.touchs; i++) {

      midi_msg_status_unpack(config["msg"][i]["pos"]["midi"]["status"].as<uint8_t>(), &status);
      slider_ptr->params.touch[i].pos.msg.type = status.type;
      slider_ptr->params.touch[i].pos.msg.data1 = config["msg"][i]["pos"]["midi"]["data1"].as<uint8_t>();
      slider_ptr->params.touch[i].pos.msg.data2 = 0;
      slider_ptr->params.touch[i].pos.msg.channel = status.channel;
      slider_ptr->params.touch[i].pos.limit.min = config["msg"][i]["pos"]["limit"]["min"].as<uint8_t>();
      slider_ptr->params.touch[i].pos.limit.max = config["msg"][i]["pos"]["limit"]["max"].as<uint8_t>();

      switch (slider_ptr->params.press) {

        case NoteOn:
          midi_msg_status_unpack(config["msg"][i]["press"]["midi"]["status"].as<uint8_t>(), &status);
          slider_ptr->params.touch[i].press.msg.type = NoteOn;
          slider_ptr->params.touch[i].press.msg.data1 = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
          slider_ptr->params.touch[i].press.msg.data2 = 0;
          slider_ptr->params.touch[i].press.msg.channel = status.channel;
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
  }
};
