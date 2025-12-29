/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

llist_t llist_mappings;

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
            mapping_ptr->continue_func_ptr(blob_ptr);
          }
          else if (blob_ptr->status == RELEASED && blob_ptr->last_status == MISSING) {
            mapping_ptr->stop_func_ptr(blob_ptr);
          }
          else if (blob_ptr->status == FREE) {
            // N/A
          }
        }
      }
      else { // RELEASESING THE BLOB OUT OF THE MAPPING
        if (blob_ptr->status == RELEASED && blob_ptr->last_status == MISSING) {
          mapping_ptr->stop_func_ptr(blob_ptr);
        }
      }
    }
  }
};

void mapping_send_midi_note_on(positon_t* positon_ptr, blob_t* blob_ptr) {
  positon_ptr->msg.type = NoteOn;
  //positon_ptr->msg.data2 = blob_ptr->centroid.z; // TODO: velocity sensing  
  positon_ptr->msg.data2 = 127;
  midi_send_out(&positon_ptr->msg);
};

void mapping_send_midi_note_off(positon_t* positon_ptr, blob_t* blob_ptr) {
  positon_ptr->msg.type = NoteOff;
  positon_ptr->msg.data2 = 0;
  midi_send_out(&positon_ptr->msg);
};

void mapping_send_midi_pos_x_msg(rect_t* bounding_box_ptr, positon_t* positon_ptr, blob_t* blob_ptr) {
  positon_ptr->last_val = positon_ptr->msg.data2;
  positon_ptr->msg.data2 = map(
    blob_ptr->centroid.x,
    bounding_box_ptr->from.x,
    bounding_box_ptr->to.x,
    positon_ptr->limit.min,
    positon_ptr->limit.max
  );
  if (positon_ptr->msg.data2 != positon_ptr->last_val) {
    midi_send_out(&positon_ptr->msg);
  }
};

void mapping_send_midi_pos_y_msg(rect_t* bounding_box_ptr, positon_t* positon_ptr, blob_t* blob_ptr) {
  positon_ptr->last_val = positon_ptr->msg.data2;
  positon_ptr->msg.data2 = map(
    blob_ptr->centroid.y,
    bounding_box_ptr->from.y,
    bounding_box_ptr->to.y,
    positon_ptr->limit.min,
    positon_ptr->limit.max
  );
  if (positon_ptr->msg.data2 != positon_ptr->last_val) {
    midi_send_out(&positon_ptr->msg);
  }
};

void mapping_send_midi_pos_z_msg(positon_t* positon_ptr, blob_t* blob_ptr) {
  positon_ptr->last_val = positon_ptr->msg.data2;
  positon_ptr->msg.data2 = map(
    blob_ptr->centroid.z,
    Z_MIN,
    Z_MAX,
    positon_ptr->limit.min,
    positon_ptr->limit.max
  );
  if (positon_ptr->msg.data2 != positon_ptr->last_val) {
    midi_send_out(&positon_ptr->msg);
  }
};
