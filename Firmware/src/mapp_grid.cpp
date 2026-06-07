/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// This GUI has been inspired by the Omnichord instrument: https://en.wikipedia.org/wiki/Omnichord

#include "mapp_grid.h"
#include "hardware_midi_io.h"

typedef struct mapp_grid_s mapp_grid_t;
struct mapp_grid_s {
  common_t common;
  grid_t params;
  uint8_t active_blob_count;
  uint8_t touch_index;
  llist_t llist_active_midi_msg;
  uint8_t active_midi_msg_count;
  midi_msg_t chord_notes[MAX_GRID_KEYS][MAX_CHORD_NOTES];
};

static mapp_grid_t mapp_grids[MAX_GRIDS];

llist_t llist_grids_pool;

bool mapping_grids_alloc(uint8_t grids_cnt) {
  if (grids_cnt <= MAX_GRIDS) {
    llist_builder(&llist_grids_pool, &mapp_grids[0], grids_cnt, sizeof(mapp_grids[0]));
    return true;
  }
  return false;
};

bool mapping_grid_is_blob_inside(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)mapping_ptr;
  if (blob_ptr->centroid.x > grid_ptr->params.rect.from.x &&
      blob_ptr->centroid.x < grid_ptr->params.rect.to.x &&
      blob_ptr->centroid.y > grid_ptr->params.rect.from.y &&
      blob_ptr->centroid.y < grid_ptr->params.rect.to.y) {
    return true;
  }
  return false;
};

bool mapping_grid_assign_blob(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)mapping_ptr;

  if (grid_ptr->touch_index < grid_ptr->params.touchs) {
    blob_ptr->action.mapping_ptr = grid_ptr;
    blob_ptr->action.touch_ptr = &grid_ptr->params.touch[grid_ptr->touch_index];
    blob_ptr->action.touch_slot = grid_ptr->touch_index;
    grid_ptr->touch_index++;
    grid_ptr->active_blob_count++;
    return true;
  }
  return false;
};

void mapping_grid_dispose_blob(void* mapping_ptr, blob_t* blob_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)mapping_ptr;

  blob_ptr->action.mapping_ptr = NULL;
  blob_ptr->action.touch_ptr = NULL;
  blob_ptr->action.touch_slot = TOUCH_SLOT_NONE;
  grid_ptr->active_blob_count--;
  if (grid_ptr->active_blob_count == 0) {
    grid_ptr->touch_index = 0;
  }
};

static uint8_t grid_key_id(mapp_grid_t* grid_ptr, blob_t* blob_ptr) {
  uint8_t key_x = constrain(
    (int)((blob_ptr->centroid.x - grid_ptr->params.rect.from.x) * grid_ptr->params.scale_factor_x),
    0, grid_ptr->params.cols - 1);
  uint8_t key_y = constrain(
    (int)((blob_ptr->centroid.y - grid_ptr->params.rect.from.y) * grid_ptr->params.scale_factor_y),
    0, grid_ptr->params.rows - 1);
  return key_y * grid_ptr->params.cols + key_x;
}

void mapping_grid_start(blob_t* blob_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)blob_ptr->action.mapping_ptr;
  touch_press_t* touch_ptr = (touch_press_t*)blob_ptr->action.touch_ptr;

  uint8_t key_id = grid_key_id(grid_ptr, blob_ptr);
  touch_ptr->press.last_val = key_id;

  if (grid_ptr->params.key[key_id].press.enabled) {
    switch (grid_ptr->params.key[key_id].press.msg.type) {
      case NoteOn:
        mapping_send_midi_note_on(&grid_ptr->params.key[key_id].press, blob_ptr);
        break;
      case MIDI_TYPE_CHORD: {
        uint8_t vel = (uint8_t)constrain((int)roundf(1.0f + constrain((blob_ptr->centroid.z - Z_MIN) / (float)(Z_MAX - Z_MIN), 0.0f, 1.0f) * 126.0f), 1, 127);
        midi_send_chord_on(grid_ptr->chord_notes[key_id], &grid_ptr->params.chord[key_id], grid_ptr->params.chan_out, vel);
        break;
      }
      default: break;
    }
  }
};

void mapping_grid_continue(blob_t* blob_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)blob_ptr->action.mapping_ptr;
  touch_press_t* touch_ptr = (touch_press_t*)blob_ptr->action.touch_ptr;

  // Compute fractional key position for hysteresis check
  float fx = constrain(
    (blob_ptr->centroid.x - grid_ptr->params.rect.from.x) * grid_ptr->params.scale_factor_x,
    0.0f, (float)(grid_ptr->params.cols - 1) + 0.9999f
  );
  float fy = constrain(
    (blob_ptr->centroid.y - grid_ptr->params.rect.from.y) * grid_ptr->params.scale_factor_y,
    0.0f, (float)(grid_ptr->params.rows - 1) + 0.9999f
  );
  uint8_t key_x = (uint8_t)fx;
  uint8_t key_y = (uint8_t)fy;
  uint8_t key_id = key_y * grid_ptr->params.cols + key_x;

  if (key_id != touch_ptr->press.last_val) {
    // Hysteresis: require blob at least 25% inside the new cell before switching.
    // Prevents chord oscillation when the centroid jitters near a cell boundary.
    float frac_x = fx - (float)key_x;
    float frac_y = fy - (float)key_y;
    const float H = 0.25f;
    if (frac_x < H || frac_x > (1.0f - H) || frac_y < H || frac_y > (1.0f - H)) return;

    const uint8_t prev_id = touch_ptr->press.last_val;

    if (grid_ptr->params.key[prev_id].press.enabled) {
      switch (grid_ptr->params.key[prev_id].press.msg.type) {
        case NoteOn: mapping_send_midi_note_off(&grid_ptr->params.key[prev_id].press); break;
        case MIDI_TYPE_CHORD: midi_send_chord_off(grid_ptr->chord_notes[prev_id], grid_ptr->params.chord[prev_id].type); break;
        default: break;
      }
    }
    if (grid_ptr->params.key[key_id].press.enabled) {
      switch (grid_ptr->params.key[key_id].press.msg.type) {
        case NoteOn: mapping_send_midi_note_on(&grid_ptr->params.key[key_id].press, blob_ptr); break;
        case MIDI_TYPE_CHORD: {
          uint8_t vel = (uint8_t)constrain((int)roundf(1.0f + constrain((blob_ptr->centroid.z - Z_MIN) / (float)(Z_MAX - Z_MIN), 0.0f, 1.0f) * 126.0f), 1, 127);
          midi_send_chord_on(grid_ptr->chord_notes[key_id], &grid_ptr->params.chord[key_id], grid_ptr->params.chan_out, vel);
          break;
        }
        default: break;
      }
    }
    touch_ptr->press.last_val = key_id;
  }
};

void mapping_grid_stop(blob_t* blob_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)blob_ptr->action.mapping_ptr;
  touch_press_t* touch_ptr = (touch_press_t*)blob_ptr->action.touch_ptr;
  const uint8_t key_id = touch_ptr->press.last_val;
  if (grid_ptr->params.key[key_id].press.enabled) {
    switch (grid_ptr->params.key[key_id].press.msg.type) {
      case NoteOn: mapping_send_midi_note_off(&grid_ptr->params.key[key_id].press); break;
      case MIDI_TYPE_CHORD: midi_send_chord_off(grid_ptr->chord_notes[key_id], grid_ptr->params.chord[key_id].type); break;
      default: break;
    }
  }
};

bool mapping_grid_hardware_midi_receive(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)mapping_ptr;
  if (midi_msg_ptr->channel == grid_ptr->params.chan_in) {
    return true;
  }
  return false;
};

void mapping_grid_hardware_midi_update(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)mapping_ptr;
  llist_push_front(&grid_ptr->llist_active_midi_msg, midi_msg_ptr);
  grid_ptr->active_midi_msg_count++;

  for (unsigned int index = 0; index < grid_ptr->params.keys; index += grid_ptr->active_midi_msg_count) {
    uint8_t inc = index;
    for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&grid_ptr->llist_active_midi_msg); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
      if (inc >= grid_ptr->params.keys) break;
      midi_msg_t* midi_msg_ptr = (midi_msg_t*)ITERATOR_DATA(node_ptr);
      grid_ptr->params.key[inc].press.msg.data1 = midi_msg_ptr->data1;
      inc++;
    }
  }
};

void mapping_grid_hardware_midi_dispose(void* mapping_ptr, midi_msg_t* midi_msg_ptr) {
  mapp_grid_t* grid_ptr = (mapp_grid_t*)mapping_ptr;
  grid_ptr->active_midi_msg_count--;
  if (grid_ptr->active_midi_msg_count == 0) {  // Save/rescue all llist nodes
    midi_msg_t* midi_msg_ptr = NULL;
    while ((midi_msg_ptr = (midi_msg_t*)llist_pop_front(&grid_ptr->llist_active_midi_msg)) != NULL) {
      llist_push_front(&llist_midi_nodes_pool, midi_msg_ptr);
    }
  }
};

void mapping_grid_create(const JsonObject &config) {

  mapp_grid_t* grid_ptr = (mapp_grid_t*)llist_pop_front(&llist_grids_pool);

  grid_ptr->common.hardware_midi_receive_func_ptr = &mapping_grid_hardware_midi_receive;
  grid_ptr->common.hardware_midi_update_func_ptr = &mapping_grid_hardware_midi_update;
  grid_ptr->common.hardware_midi_dispose_func_ptr = &mapping_grid_hardware_midi_dispose;

  grid_ptr->common.is_blob_inside_func_ptr = &mapping_grid_is_blob_inside;
  grid_ptr->common.blob_assign_func_ptr = &mapping_grid_assign_blob;
  grid_ptr->common.blob_dispose_func_ptr = &mapping_grid_dispose_blob;

  grid_ptr->common.start_func_ptr = &mapping_grid_start;
  grid_ptr->common.continue_func_ptr = &mapping_grid_continue;
  grid_ptr->common.stop_func_ptr = &mapping_grid_stop;

  grid_ptr->params.touchs = config["touchs"] | MAX_GRID_TOUCHS;
  grid_ptr->params.rect.from.x = config["from"][0].as<float>();
  grid_ptr->params.rect.from.y = config["from"][1].as<float>();
  grid_ptr->params.rect.to.x = config["to"][0].as<float>();
  grid_ptr->params.rect.to.y = config["to"][1].as<float>();
  grid_ptr->params.cols = config["cols"].as<uint8_t>();
  grid_ptr->params.rows = config["rows"].as<uint8_t>();
  grid_ptr->params.chan_in  = config["chan"]["in"].as<uint8_t>();
  grid_ptr->params.chan_out = config["chan"]["out"].as<uint8_t>();
  
  grid_ptr->params.keys = grid_ptr->params.cols * grid_ptr->params.rows;

  if (grid_ptr->params.keys <= MAX_GRID_KEYS) {

    // Pre-compute grid size
    float grid_size_x = grid_ptr->params.rect.to.x - grid_ptr->params.rect.from.x;
    float grid_size_y = grid_ptr->params.rect.to.y - grid_ptr->params.rect.from.y;
    if (grid_size_x == 0.0f || grid_size_y == 0.0f) return;
    // Pre-compute scale factors
    grid_ptr->params.scale_factor_x = grid_ptr->params.cols / grid_size_x;
    grid_ptr->params.scale_factor_y = grid_ptr->params.rows / grid_size_y;
  
    for (uint8_t i = 0; i < grid_ptr->params.keys; i++) {
      midi_status_t status;
      if (config["msg"][i]["press"]["chord"].is<JsonVariant>()) {
        grid_ptr->params.key[i].press.msg.type = MIDI_TYPE_CHORD;
        grid_ptr->params.key[i].press.enabled  = config["msg"][i]["press"]["enabled"] | true;
        grid_ptr->params.chord[i].type         = config["msg"][i]["press"]["chord"].as<uint8_t>();
        grid_ptr->params.chord[i].note         = config["msg"][i]["press"]["note"].as<uint8_t>();
      } else {
        uint8_t press_status = config["msg"][i]["press"]["midi"]["status"].as<uint8_t>();
        midi_msg_status_unpack(press_status, &status);
        grid_ptr->params.key[i].press.enabled     = (press_status != 0) && (config["msg"][i]["press"]["enabled"] | true);
        grid_ptr->params.key[i].press.msg.type    = status.type;
        grid_ptr->params.key[i].press.msg.data1   = config["msg"][i]["press"]["midi"]["data1"].as<uint8_t>();
        grid_ptr->params.key[i].press.msg.data2   = 127;
        grid_ptr->params.key[i].press.msg.channel = status.channel;
        grid_ptr->params.key[i].press.limit.min   = config["msg"][i]["press"]["limit"]["min"].as<uint8_t>();
        grid_ptr->params.key[i].press.limit.max   = config["msg"][i]["press"]["limit"]["max"].as<uint8_t>();
      }
    }
    llist_push_back(&llist_mappings, grid_ptr);
  }
};
