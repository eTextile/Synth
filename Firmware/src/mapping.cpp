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
            Serial.printf("\n_BLOB_ASSIGN_START");
          }
        }
        else {
          if (blob_ptr->status == NEW) {
            mapping_ptr->start_func_ptr(blob_ptr);
            Serial.printf("\n_BLOB_START");
          }
          else if (blob_ptr->status == PRESENT) {
            mapping_ptr->continue_func_ptr(blob_ptr);
            Serial.printf("\n_BLOB_PRESENT");
          }
          else if (blob_ptr->status == RELEASED && blob_ptr->last_status == MISSING) { // FIXME
            mapping_ptr->stop_func_ptr(blob_ptr);
            Serial.printf("\n_BLOB_RELEASED_IN");
          }
          else if (blob_ptr->status == FREE) {
            common_t* mapping_ptr = (common_t*)blob_ptr->action.mapping_ptr;
            if (mapping_ptr) mapping_ptr->blob_dispose_func_ptr(mapping_ptr, blob_ptr);
            Serial.printf("\n_BLOB_FREE");
          }
        }
      }
      else { // RELEASESING THE BLOB OUT OF THE MAPPING
        if (blob_ptr->status == RELEASED && blob_ptr->last_status == MISSING) {
          mapping_ptr->stop_func_ptr(blob_ptr);
          Serial.printf("\n_BLOB_RELEASED_OUT");
        }
      }
    }
  }
};
