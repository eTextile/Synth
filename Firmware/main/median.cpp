/*
  median filter (c) 2019 by RainerU
  This code is under the MIT License
  https://github.com/RainerU/medianfilter

  Added multy blob filtering capability
  Copyright (c) 2021 Maurin Donneaud <maurin@etextile.org>
*/

#include "median.h"

void median(llist_t* blobs_ptr, median_t* median_ptr) {

  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {

    float inputVal = blob_ptr->box.D; // The new value
    float outputVal = inputVal;       // The new value could be the median

    // Initialize all arrays
    if (blob_ptr->alive && !blob_ptr->lastState) {
      memset(median_ptr[blob_ptr->UID].zVal, blob_ptr->box.D, MEDIAN_TIME_WINDOW * sizeof(uint8_t));
      memset(median_ptr[blob_ptr->UID].zOrd, blob_ptr->box.D, MEDIAN_TIME_WINDOW * sizeof(uint8_t));
      median_ptr[blob_ptr->UID].index = 0;
    }

    else { // Store new value
      if (++median_ptr[blob_ptr->UID].index >= MEDIAN_TIME_WINDOW) median_ptr[blob_ptr->UID].index = 0; // One step forward in ring storage

      float lastZ = median_ptr[blob_ptr->UID].zVal[median_ptr[blob_ptr->UID].index];      // Save last value
      uint8_t lastOrdZ = median_ptr[blob_ptr->UID].zOrd[median_ptr[blob_ptr->UID].index]; // Save last order

      median_ptr[blob_ptr->UID].zVal[median_ptr[blob_ptr->UID].index] = blob_ptr->box.D;  // Store new value
      median_ptr[blob_ptr->UID].zOrd[median_ptr[blob_ptr->UID].index] = 0;                // Reset order number for new value

      uint8_t index = median_ptr[blob_ptr->UID].index;                                    // Get index
      if (++index >= MEDIAN_TIME_WINDOW) index = 0;                                       // loop through array storage

      do {
        if (blob_ptr->box.D <= median_ptr[blob_ptr->UID].zVal[index] &&
            lastOrdZ > median_ptr[blob_ptr->UID].zOrd[index]) {
          median_ptr[blob_ptr->UID].zOrd[index]++;     // Remove bigger value, add smaller value
        }
        else if (blob_ptr->box.D > median_ptr[blob_ptr->UID].zVal[index] &&
                 lastOrdZ < median_ptr[blob_ptr->UID].zOrd[index]) {
          median_ptr[blob_ptr->UID].zOrd[index]--;                            // Remove smaller value, add bigger value
        }
        if (median_ptr[blob_ptr->UID].zOrd[index] == MEDIAN_POS) {
          outputVal = median_ptr[blob_ptr->UID].zVal[index];                  // Median found
        }
        if (blob_ptr->box.D > median_ptr[blob_ptr->UID].zVal[index]) {
          median_ptr[blob_ptr->UID].zOrd[median_ptr[blob_ptr->UID].index]++;  // Compute new value order
        }
        if (++index >= MEDIAN_TIME_WINDOW) index = 0;                         // Go one step in ring storage
      } while (index != median_ptr[blob_ptr->UID].index);                     // Stop at index position
    }
    blob_ptr->box.D = outputVal;                                              // Replace the value with the computed median value
    Serial.printf("\n%f ; %f", inputVal, outputVal);
  }
}
