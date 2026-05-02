/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

llist_t llist_mappings;

// Sends the deferred NoteOn once attack_done is true.
// touch_1d_t / touch_2d_t / touch_3d_t all have press as their first member,
// so a touch_1d_t* cast is safe for accessing the press axis regardless of TUI type.
#if defined(BLOB_VELOCITY)
static void mapping_flush_pending_note_on(blob_t* blob_ptr) {
  if (!blob_ptr->action.note_on_z_pending || !blob_ptr->velocity.attack_done) return;
  if (blob_ptr->action.touch_ptr == NULL) return;
  axis_t* axis_ptr = &((touch_1d_t*)blob_ptr->action.touch_ptr)->press;
  float scaled = blob_ptr->velocity.attack_z / (float)VELOCITY_ATTACK_Z_MAX;
  axis_ptr->msg.data2 = (uint8_t)constrain(
    (int)(scaled * (axis_ptr->limit.max - axis_ptr->limit.min) + axis_ptr->limit.min),
    axis_ptr->limit.min, axis_ptr->limit.max
  );
  llist_push_front(&llist_midi_out, &axis_ptr->msg);
  blob_ptr->action.note_on_z_pending = false;
}

// Sends the deferred NoteOn for ROL sliders once the first xy velocity sample is ready
// (xy_time_stamp advances past born_at on the first PRESENT frame, i.e. ~1ms after NEW).
static void mapping_flush_pending_note_on_xy(blob_t* blob_ptr) {
  if (!blob_ptr->action.note_on_xy_pending) return;
  if (blob_ptr->action.touch_ptr == NULL) return;
  if (blob_ptr->velocity.xy_time_stamp == blob_ptr->velocity.born_at) return; // no sample yet
  axis_t* axis_ptr = &((touch_1d_t*)blob_ptr->action.touch_ptr)->press;
  float scaled = blob_ptr->velocity.xy / (float)VELOCITY_XY_MAX;
  axis_ptr->msg.data2 = (uint8_t)constrain(
    (int)(scaled * (axis_ptr->limit.max - axis_ptr->limit.min) + axis_ptr->limit.min),
    axis_ptr->limit.min, axis_ptr->limit.max
  );
  llist_push_front(&llist_midi_out, &axis_ptr->msg);
  blob_ptr->action.note_on_xy_pending = false;
}
#endif

void mapping_lib_update(void) {

  for (lnode_t* mapping_node_ptr = ITERATOR_START_FROM_HEAD(&llist_mappings); mapping_node_ptr != NULL; mapping_node_ptr = ITERATOR_NEXT(mapping_node_ptr)) {
    common_t* mapping_ptr = (common_t*)ITERATOR_DATA(mapping_node_ptr);
    
    for (lnode_t* blob_node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs); blob_node_ptr != NULL; blob_node_ptr = ITERATOR_NEXT(blob_node_ptr)) {
      blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(blob_node_ptr);

      if (mapping_ptr->is_blob_inside_func_ptr(mapping_ptr, blob_ptr)) {

        if (blob_ptr->action.mapping_ptr == NULL) {
          if (mapping_ptr->blob_assign_func_ptr(mapping_ptr, blob_ptr)) {
            mapping_ptr->start_func_ptr(blob_ptr);
          }
        }
        else {
          if (blob_ptr->status == NEW) {
            mapping_ptr->start_func_ptr(blob_ptr);
          }
          else if (blob_ptr->status == PRESENT) {
            #if defined(BLOB_VELOCITY)
            mapping_flush_pending_note_on(blob_ptr);
            mapping_flush_pending_note_on_xy(blob_ptr);
            #endif
            mapping_ptr->continue_func_ptr(blob_ptr);
          }
          else if (blob_ptr->status == RELEASED && blob_ptr->last_status == MISSING) {
            mapping_ptr->stop_func_ptr(blob_ptr);
          }
        }
      }
      else { // RELEASESING THE BLOB OUT OF THE MAPPING
        if (blob_ptr->action.mapping_ptr != NULL) {
          if (blob_ptr->status == RELEASED && blob_ptr->last_status == MISSING) {
            mapping_ptr->stop_func_ptr(blob_ptr);
          }
        }
      }
    }
  }
};

// Called from every TUI _start when press mode == NoteOn.
// When BLOB_VELOCITY is defined: arms the deferred NoteOn (actual send happens in
// mapping_flush_pending_note_on, called from the PRESENT branch of the dispatcher).
// When BLOB_VELOCITY is not defined: sends immediately using centroid.z as velocity.
void mapping_send_midi_note_on(axis_t* axis_ptr, blob_t* blob_ptr) {
  axis_ptr->msg.type = NoteOn;
#if defined(BLOB_VELOCITY)
  blob_ptr->action.note_on_z_pending = true; // flushed by mapping_flush_pending_note_on()
#else
  axis_ptr->msg.data2 = (uint8_t)map(blob_ptr->centroid.z, Z_MIN, Z_MAX, axis_ptr->limit.min, axis_ptr->limit.max);
  llist_push_front(&llist_midi_out, &axis_ptr->msg);
#endif
};

// For MOVE_ROL sliders: velocity comes from lateral sliding speed, not Z attack.
// Sends immediately — no deferred attack window needed for a rolling gesture.
void mapping_send_midi_note_on_xy(axis_t* axis_ptr, blob_t* blob_ptr) {
  axis_ptr->msg.type = NoteOn;
  float scaled = blob_ptr->velocity.xy / (float)VELOCITY_XY_MAX;
  int val = (int)(scaled * (axis_ptr->limit.max - axis_ptr->limit.min) + axis_ptr->limit.min);
  axis_ptr->msg.data2 = (uint8_t)constrain(max(val, 1), axis_ptr->limit.min, axis_ptr->limit.max);
  llist_push_front(&llist_midi_out, &axis_ptr->msg);
  blob_ptr->action.note_on_xy_pending = false; // cancel deferred NoteOn if step changed first
};

void mapping_send_midi_note_off(axis_t* axis_ptr) {
  axis_ptr->msg.type = NoteOff;
  axis_ptr->msg.data2 = 0;
  llist_push_front(&llist_midi_out, &axis_ptr->msg);
};

void mapping_send_midi_msg_pos_x(rect_t* bounding_box_ptr, axis_t* axis_ptr, blob_t* blob_ptr) {
  axis_ptr->msg.data2 = map(
    blob_ptr->centroid.x,
    bounding_box_ptr->to.x,
    bounding_box_ptr->from.x,
    axis_ptr->limit.max,
    axis_ptr->limit.min
  );
  if (axis_ptr->msg.data2 != axis_ptr->last_val) {
    if ((millis() - axis_ptr->midi_time_stamp) > MIDI_THROTTLE_MS) {
      llist_push_front(&llist_midi_out, &axis_ptr->msg);
      axis_ptr->last_val = axis_ptr->msg.data2;
      axis_ptr->midi_time_stamp = millis();
    }
  }
};

void mapping_send_midi_msg_pos_y(rect_t* bounding_box_ptr, axis_t* axis_ptr, blob_t* blob_ptr) {
  axis_ptr->msg.data2 = map(
    blob_ptr->centroid.y,
    bounding_box_ptr->from.y,
    bounding_box_ptr->to.y,
    axis_ptr->limit.min,
    axis_ptr->limit.max
  );
  if (axis_ptr->msg.data2 != axis_ptr->last_val) {
    if ((millis() - axis_ptr->midi_time_stamp) > MIDI_THROTTLE_MS) {
      llist_push_front(&llist_midi_out, &axis_ptr->msg);
      axis_ptr->last_val = axis_ptr->msg.data2;
      axis_ptr->midi_time_stamp = millis();
    }
  }
};

void mapping_send_midi_msg_press(axis_t* axis_ptr, blob_t* blob_ptr) {
  axis_ptr->msg.data2 = map(
    blob_ptr->centroid.z,
    Z_MIN,
    Z_MAX,
    axis_ptr->limit.min,
    axis_ptr->limit.max
  );
  if (axis_ptr->msg.data2 != axis_ptr->last_val) {
    if ((millis() - axis_ptr->midi_time_stamp) > MIDI_THROTTLE_MS) {
      llist_push_front(&llist_midi_out, &axis_ptr->msg);
      axis_ptr->last_val = axis_ptr->msg.data2;
      axis_ptr->midi_time_stamp = millis();
    }
  }
};
