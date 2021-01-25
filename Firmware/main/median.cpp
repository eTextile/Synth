/*
  median filter (c) 2019 by RainerU
  This code is under the MIT License
  https://github.com/RainerU/medianfilter

  Added multy blob filtering capability
  Copyright (c) 2021 Maurin Donneaud <maurin@etextile.org>
*/

#include "median.h"

void median(llist_t* blobs_ptr, median_t* storage_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {

    float inputVal = blob_ptr->box.D; // The new value
    float outputVal = inputVal;       // The new value could be the median

    // Initialize all arrays
    if (blob_ptr->alive && storage_ptr[blob_ptr->UID].init) {
      storage_ptr[blob_ptr->UID].init = false;                    // Run initialization once
      for (int i = 0; i < TIME_WINDOW; i++) {
        storage_ptr[blob_ptr->UID].val[i] = blob_ptr->box.D;      // Fill array with new value
        storage_ptr[blob_ptr->UID].sort[i] = blob_ptr->box.D;     // Fill array with new value
      }
    }
    // Store new value
    else {
      if (storage_ptr[blob_ptr->UID].index++ >= TIME_WINDOW) storage_ptr[blob_ptr->UID].index = 0;  // Go one step in ring storage
      float oldVal = storage_ptr[blob_ptr->UID].val[storage_ptr[blob_ptr->UID].index];              // Save old value
      float oldOrd = storage_ptr[blob_ptr->UID].sort[storage_ptr[blob_ptr->UID].index];             // Save old order number
      storage_ptr[blob_ptr->UID].val[storage_ptr[blob_ptr->UID].index] = blob_ptr->box.D;           // Store new value
      storage_ptr[blob_ptr->UID].sort[storage_ptr[blob_ptr->UID].index] = 0;                        // Reset order number for new value

      uint16_t c = storage_ptr[blob_ptr->UID].index;                            // Get index
      if (++c >= TIME_WINDOW) c = 0;                                            // loop through array storage

      do {
        if (blob_ptr->box.D <= storage_ptr[blob_ptr->UID].val[c] &&
            oldOrd > storage_ptr[blob_ptr->UID].sort[c])
          storage_ptr[blob_ptr->UID].sort[c]++;                                 // Remove bigger value, add smaller value
        else if (blob_ptr->box.D > storage_ptr[blob_ptr->UID].val[c] &&
                 oldOrd < storage_ptr[blob_ptr->UID].sort[c])
          storage_ptr[blob_ptr->UID].sort[c]--;                                 // Remove smaller value, add bigger value
        if (storage_ptr[blob_ptr->UID].sort[c] == MEDIAN_POS)
          outputVal = storage_ptr[blob_ptr->UID].val[c];                        // Median found
        if (blob_ptr->box.D > storage_ptr[blob_ptr->UID].val[c])
          storage_ptr[blob_ptr->UID].sort[storage_ptr[blob_ptr->UID].index]++;  // Compute new value order
        if (++c >= TIME_WINDOW) c = 0;                                          // Go one step in ring storage
      } while (c != storage_ptr[blob_ptr->UID].index);                          // Stop at index position
    }

    if (!blob_ptr->alive) {
      storage_ptr[blob_ptr->UID].init = true;                                   // Can run initialization again
    }
    blob_ptr->box.D = outputVal;                                                // Replace the value with the computed median value
    //Serial.printf("\n%f ; %f", inputVal, outputVal);
  }
}
