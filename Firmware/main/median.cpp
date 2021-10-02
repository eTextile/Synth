/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// https://github.com/RobTillaart/RunningMedian/blob/2a155ba97f1d393077590decc71f770f1962a994/RunningMedian.cpp

#include "median.h"

median_t filter[MAX_SYNTH] = {0};  // 1D ...

// resets all internal counters
void RUNING_MEDIAN_SETUP(void) {
  for (uint8_t i = 0; i < MAX_SYNTH; i++) {
    for (uint8_t j = 0; j < M_WINDOW; j++) {
      filter[i].X_sort[j] = j;
      filter[i].Y_sort[j] = j;
    };
  };
};

// Adds a new value to the data-set
// or overwrites the oldest if buffer is full
void runing_median(void) {

  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {

    if (!blob_ptr->lastState) {
      filter[blob_ptr->UID].count = 0;
      filter[blob_ptr->UID].index = 0;
    };

    // Add the new values to the input runing buffer
    filter[blob_ptr->UID].X_rawVal[filter[blob_ptr->UID].index] = blob_ptr->centroid.X;
    filter[blob_ptr->UID].Y_rawVal[filter[blob_ptr->UID].index] = blob_ptr->centroid.Y;

    // Sort the [X] values
    for (uint8_t i = 1; i <= filter[blob_ptr->UID].count; i++) {
      uint8_t j = i;
      uint8_t temp = filter[blob_ptr->UID].X_sort[j];
      while ((j > 0) && (filter[blob_ptr->UID].X_rawVal[temp] < filter[blob_ptr->UID].X_rawVal[filter[blob_ptr->UID].X_sort[j - 1]])) {
        filter[blob_ptr->UID].X_sort[j] = filter[blob_ptr->UID].X_sort[j - 1];
        j--;
      };
      filter[blob_ptr->UID].X_sort[j] = temp;
    };

    // Sort the [Y] values
    for (uint8_t i = 1; i <= filter[blob_ptr->UID].count; i++) {
      uint8_t j = i;
      uint8_t temp = filter[blob_ptr->UID].Y_sort[j];
      while ((j > 0) && (filter[blob_ptr->UID].Y_rawVal[temp] < filter[blob_ptr->UID].Y_rawVal[filter[blob_ptr->UID].Y_sort[j - 1]])) {
        filter[blob_ptr->UID].Y_sort[j] = filter[blob_ptr->UID].Y_sort[j - 1];
        j--;
      };
      filter[blob_ptr->UID].Y_sort[j] = temp;
    };

    float sum_X = 0;
    float sum_Y = 0;

    for (uint8_t i = 0; i < filter[blob_ptr->UID].count; i++) {
      sum_X += filter[blob_ptr->UID].X_rawVal[i];
      sum_Y += filter[blob_ptr->UID].Y_rawVal[i];
    };

    //blob_ptr->centroid.X = sum_X / filter[blob_ptr->UID].count;
    //blob_ptr->centroid.Y = sum_Y / filter[blob_ptr->UID].count;

    if (filter[blob_ptr->UID].count < M_WINDOW) {
      filter[blob_ptr->UID].count++;
    };

    if (filter[blob_ptr->UID].index < M_WINDOW) {
      filter[blob_ptr->UID].index++;
    } else {
      filter[blob_ptr->UID].index = 0;
    };

  };
};
