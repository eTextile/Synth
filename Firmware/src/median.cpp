/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// Blob centroid smoothing using an Exponential Moving Average (EMA).
//
// Each frame: smooth = α * new_position + (1-α) * previous_smooth
// On first contact the filter is seeded with the raw position so there is no
// initial lag. On MISSING / FREE the centroid is left untouched so that
// mapping layers can still read the last valid position for note-off handling.
//
// Tune EMA_ALPHA_POSITION in median.h:
//   lower α → smoother trajectory, more lag
//   higher α → less lag, more noise

#include "median.h"

median_t filter[MAX_BLOBS];

void running_median_setup(void) {
  for (uint8_t i = 0; i < MAX_BLOBS; i++) {
    filter[i].x = 0.0f;
    filter[i].y = 0.0f;
  }
}

void runing_median(void) {
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
    uint8_t uid = blob_ptr->UID;

    if (blob_ptr->status == NEW ||
        (blob_ptr->status == PRESENT && blob_ptr->last_status == MISSING)) {
      // Seed filter with first measured position — no lag on initial contact.
      filter[uid].x = blob_ptr->centroid.x;
      filter[uid].y = blob_ptr->centroid.y;
    }
    else if (blob_ptr->status == PRESENT) {
      filter[uid].x = EMA_ALPHA_POSITION * blob_ptr->centroid.x + (1.0f - EMA_ALPHA_POSITION) * filter[uid].x;
      filter[uid].y = EMA_ALPHA_POSITION * blob_ptr->centroid.y + (1.0f - EMA_ALPHA_POSITION) * filter[uid].y;
      blob_ptr->centroid.x = filter[uid].x;
      blob_ptr->centroid.y = filter[uid].y;
    }
    // MISSING and FREE: don't overwrite centroid — mapping layers still need
    // the last valid position for note-off / release handling.
  }
}
