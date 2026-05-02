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
  midi_msg_t note_off_msgs[MAX_SLIDER_TOUCHS];
};

static mapp_slider_t mapp_sliders[MAX_SLIDERS];

llist_t llist_sliders_pool;

// Initialises the slider pool with sliders_cnt nodes carved from the static array.
// Must be called once at boot before any mapping_slider_create() calls.
// Returns false if sliders_cnt exceeds MAX_SLIDERS.
bool mapping_sliders_alloc(uint8_t sliders_cnt) {
  if (sliders_cnt < MAX_SLIDERS) {
    llist_builder(&llist_sliders_pool, &mapp_sliders[0], sliders_cnt, sizeof(mapp_sliders[0]));
    return true;
  }
  return false;
};

// Returns true if the blob centroid falls strictly inside the slider bounding rectangle.
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

// Claims the next free touch slot for this blob.
// touch_index advances sequentially; resets to 0 when all touches are released.
// Returns false when all configured touch slots are already occupied.
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

// Releases a blob from its touch slot and decrements the active count.
// Resets touch_index to 0 once the last active touch lifts so the next
// incoming blob starts from slot 0 again.
void mapping_slider_dispose_blob(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;

  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.touch_ptr = NULL;
  slider_ptr->active_blob_count--;
  if (slider_ptr->active_blob_count == 0) {
    slider_ptr->touch_index = 0;
  }
};

// Called on the first frame a blob is detected inside the slider (status == NEW).
// For MOVE_ROL: maps the initial touch position to a step index, sets the note,
// and defers the NoteOn until the xy velocity sample is ready (note_on_xy_pending).
// For other press types: fires the MIDI press message immediately.
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

  if (slider_ptr->params.press == NoteOn) {
    if (slider_ptr->params.move == MOVE_ROL) {
      touch_ptr->press.msg.type = NoteOn;
      blob_ptr->action.note_on_xy_pending = true; // flushed by mapping_flush_pending_note_on_xy()
    } else {
      mapping_send_midi_note_on(&touch_ptr->press, blob_ptr);
    }
  } else {
    mapping_send_midi_msg_press(&touch_ptr->press, blob_ptr);
  }
};

// Called every frame while the blob remains inside the slider (status == PRESENT).
// For MOVE_ROL: sends NoteOff/NoteOn pairs when the finger crosses a step boundary.
// For other move modes: streams continuous position (CC/aftertouch) along the
// slider axis (X for HORIZONTAL, Y for VERTICAL) plus optional pressure.
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
      uint8_t touch_idx = (uint8_t)(touch_ptr - slider_ptr->params.touch);
      slider_ptr->note_off_msgs[touch_idx] = touch_ptr->press.msg;
      slider_ptr->note_off_msgs[touch_idx].type = NoteOff;
      slider_ptr->note_off_msgs[touch_idx].data2 = 0;
      llist_push_front(&llist_midi_out, &slider_ptr->note_off_msgs[touch_idx]);
      touch_ptr->press.msg.data1 = new_note;
      mapping_send_midi_note_on_xy(&touch_ptr->press, blob_ptr);
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

// Called when the blob leaves the slider or is lost (status == RELEASED).
// Sends NoteOff for NoteOn press mode; other press types tail off naturally.
void mapping_slider_stop(blob_t* blob_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)blob_ptr->action.mapping_ptr;
  touch_2d_t* touch_ptr = (touch_2d_t*)blob_ptr->action.touch_ptr;
  if (slider_ptr->params.press == NoteOn) {
    mapping_send_midi_note_off(&touch_ptr->press);
  }
};

// Returns true if the incoming hardware MIDI message is on the slider's input channel,
// qualifying it for step-note population via mapping_slider_hardware_midi_update().
bool mapping_slider_hardware_midi_receive(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;
  if (midi_msg_ptr->channel == slider_ptr->params.input_chan) {
    return true;
  }
  return false;
};

// Populates the slider's step_note table from a hardware MIDI NoteOn received on input_chan.
// Each populate mode distributes the incoming notes differently across the step array:
//   POPULATE_AS_PLAYED  — fills steps in arrival order (wraps around).
//   POPULATE_UP         — re-sorts all held notes ascending after each new arrival.
//   POPULATE_DOWN       — re-sorts all held notes descending after each new arrival.
//   POPULATE_OCTAVE     — fills steps with a chromatic run starting from the received note.
//   POPULATE_PING_PONG  — bounces the fill position back and forth across the step range.
void mapping_slider_hardware_midi_update(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;
  llist_push_front(&slider_ptr->llist_active_midi_msg, midi_msg_ptr);
  slider_ptr->active_midi_msg_count++;

  uint8_t steps = slider_ptr->params.steps;
  if (steps == 0) return;

  switch (slider_ptr->params.populate) {

    case POPULATE_OFF:
      break;

    case POPULATE_AS_PLAYED:
      uint8_t zone = (slider_ptr->active_midi_msg_count - 1) % steps;
      slider_ptr->params.step_note[zone] = midi_msg_ptr->data1;
      break;

    case POPULATE_UP:
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

    case POPULATE_DOWN:
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

    case POPULATE_OCTAVE:
      uint8_t base = midi_msg_ptr->data1;
      for (uint8_t i = 0; i < steps; i++) {
        slider_ptr->params.step_note[i] = base + i;
      }
      break;

    case POPULATE_PING_PONG:
      if (steps > 1) {
        uint8_t period = 2 * (steps - 1);
        uint8_t pos = (slider_ptr->active_midi_msg_count - 1) % period;
        uint8_t zone = (pos < steps) ? pos : period - pos;
        slider_ptr->params.step_note[zone] = midi_msg_ptr->data1;
      }
      break;
  }
};

// Called when a hardware MIDI NoteOff is received for a previously tracked note.
// Decrements the active count; when all notes are released the active message list
// is drained and its nodes returned to the shared MIDI pool for reuse.
void mapping_slider_hardware_midi_dispose(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_slider_t* slider_ptr = (mapp_slider_t*)mapping_ptr;
  slider_ptr->active_midi_msg_count--;
  if (slider_ptr->active_midi_msg_count == 0) {  // Save/rescue all llist nodes
    midi_msg_t* midi_msg_ptr = NULL;
    while ((midi_msg_ptr = (midi_msg_t*)llist_pop_front(&slider_ptr->llist_active_midi_msg)) != NULL) {
      llist_push_front(&llist_midi_nodes_pool, midi_msg_ptr);
    }
  }
};

// Deserialises one slider entry from the JSON config, wires all function pointers,
// and appends the fully configured slider to llist_mappings.
// dir (VERTICAL / HORIZONTAL) is inferred from the bounding-box aspect ratio.
// step_note[] is pre-filled with a chromatic run starting at MIDI note 60.
void mapping_slider_create(const JsonObject &config) {

  mapp_slider_t* slider_ptr = (mapp_slider_t*)llist_pop_front(&llist_sliders_pool);

  slider_ptr->common.midi_hardware_receive_func_ptr = &mapping_slider_hardware_midi_receive;
  slider_ptr->common.midi_hardware_update_func_ptr = &mapping_slider_hardware_midi_update;
  slider_ptr->common.midi_hardware_dispose_func_ptr = &mapping_slider_hardware_midi_dispose;

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
  slider_ptr->params.input_chan = config["input_chan"].as<uint8_t>();

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
          slider_ptr->params.touch[i].press.limit.min = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
          slider_ptr->params.touch[i].press.limit.max = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
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
