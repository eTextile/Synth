/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/src/omv/imlib/blob.c
    - This file is part of the OpenMV project.
    - Copyright (c) 2013- Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-Synthesizer project - http://synth.eTextile.org
    - Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "blob.h"

#define MAX_BLOBS           32            // [1:64] Set how many blobs can be tracked at the same time
#define LIFO_NODES          512           // Set the maximum nodes number
#define X_STRIDE            4             // Speed up X scanning
#define Y_STRIDE            1             // Speed up Y scanning
#define MIN_BLOB_PIX        4             // Set the minimum blob pixels
#define DEBOUNCE_TIME       15            // Avioding undesired bouncing effect when taping on the sensor

uint8_t bitmapArray[NEW_FRAME] = {0};     // 1D Array to store (64*64) binary values
xylr_t lifoArray[LIFO_NODES] = {0};       // 1D Array to store lifo nodes
blob_t blobArray[MAX_BLOBS] = {0};        // 1D Array to store blobs
velocity_t blobVelocity[MAX_SYNTH] = {0}; // 1D Array to store XY & Z blobs velocity
point_t lastCoord[MAX_SYNTH] = {0};

llist_t llist_context_stack;              // Free nodes stack
llist_t llist_context;                    // Used nodes
llist_t llist_blobs_stack;                // Free nodes stack
llist_t llist_blobs_temp;                 // Intermediate blobs linked list
llist_t blobs;                            // Output blobs linked list

void llist_lifo_init(llist_t* llist_ptr, xylr_t* nodesArray_ptr, const int nodes) {
  llist_raz(llist_ptr);
  for (int i = 0; i < nodes; i++) {
    llist_push_front(llist_ptr, &nodesArray_ptr[i]);
  };
};

void llist_blob_init(llist_t* llist_ptr, blob_t* nodesArray_ptr, const int nodes) {
  llist_raz(llist_ptr);
  for (int i = 0; i < nodes; i++) {
    llist_push_front(llist_ptr, &nodesArray_ptr[i]);
  };
};

void BLOB_SETUP(void) {
  llist_lifo_init(&llist_context_stack, &lifoArray[0], LIFO_NODES); // Add X nodes to the llist_context_stack
  llist_blob_init(&llist_blobs_stack, &blobArray[0], MAX_BLOBS);    // Add X nodes to the llist_blobs_stack
  llist_raz(&llist_context);
  llist_raz(&llist_blobs_temp);
  llist_raz(&blobs);
};

/////////////////////////////// Scanline flood fill algorithm / SFF
/////////////////////////////// Connected-component labeling / CCL
void find_blobs(void) {

  memset((uint8_t*)bitmapArray, 0, SIZEOF_FRAME);

  for (uint8_t posY = 0; posY < NEW_ROWS; posY += Y_STRIDE) {

    uint8_t* row_ptr_A = COMPUTE_IMAGE_ROW_PTR(&interpFrame, posY);
    uint8_t* bmp_row_ptr_A = &bitmapArray[0] + posY * NEW_COLS;

    for (uint8_t posX = (posY % X_STRIDE); posX < NEW_COLS; posX += X_STRIDE) {
      if (!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_A, posX) &&
          PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_A, posX), presets[THRESHOLD].val)
         ) {

        uint8_t oldX = posX;
        uint8_t oldY = posY;

        uint8_t blob_x1 = posX;
        uint8_t blob_x2 = posX;

        uint8_t blob_height = 0;
        uint8_t blob_depth = 0;

        uint16_t blob_pixels = 0;
        float blob_cx = 0;
        float blob_cy = 0;

        while (1) { // while_A
          uint8_t left = posX;
          uint8_t right = posX;

          uint8_t* row_ptr_B = COMPUTE_IMAGE_ROW_PTR(&interpFrame, posY);
          uint8_t* bmp_row_ptr_B = &bitmapArray[0] + posY * NEW_COLS;

          while ((left > 0) &&
                 !IMAGE_GET_PIXEL_FAST(bmp_row_ptr_B, left - 1) &&
                 PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, left - 1), presets[THRESHOLD].val)
                ) {
            left--;
          }
          while (right < (NEW_COLS - 1) &&
                 !IMAGE_GET_PIXEL_FAST(bmp_row_ptr_B, right + 1) &&
                 PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, right + 1), presets[THRESHOLD].val)
                ) {
            right++;
          }

          blob_x1 = MIN(blob_x1, left);
          blob_x2 = MAX(blob_x2, right);

          for (int i = left; i <= right; i++) {
            IMAGE_SET_PIXEL_FAST(bmp_row_ptr_B, i, 1);
            blob_depth = MAX(blob_depth, IMAGE_GET_PIXEL_FAST(row_ptr_B, i));
          }

          uint16_t sum = ((right * (right + 1)) - (left * (left - 1))) / 2;

          uint8_t pixels = right - left + 1;
          blob_pixels += pixels;
          blob_cx += sum;
          blob_cy += posY * pixels;

          uint8_t top_left = left;
          uint8_t bot_left = left;

          boolean break_out = false;

          while (1) { // while_B

            if (posY > 0) {
              row_ptr_B = COMPUTE_IMAGE_ROW_PTR(&interpFrame, posY - 1);
              bmp_row_ptr_B = &bitmapArray[0] + (posY - 1) * NEW_COLS;

              boolean recurse = true;
              for (uint8_t i = top_left; i <= right; i++) {

                if ((!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_B, i))
                    && (PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), presets[THRESHOLD].val))) {

                  xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_stack);
                  context->x = posX;
                  context->y = posY;
                  context->l = left;
                  context->r = right;
                  context->t_l = i++; // Don't test the same pixel again
                  context->b_l = bot_left;
                  llist_push_front(&llist_context, context);
                  posX = i;
                  posY--;
                  recurse = false;
                  break;
                }
              }
              if (!recurse) {
                break;
              }
            }

            row_ptr_B = COMPUTE_IMAGE_ROW_PTR(&interpFrame, posY + 1);
            bmp_row_ptr_B = &bitmapArray[0] + (posY + 1) * NEW_COLS;

            boolean recurse = true;
            for (uint8_t i = bot_left; i <= right; i++) {

              if (!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_B, i) &&
                  PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), presets[THRESHOLD].val)) {

                xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_stack);
                context->x = posX;
                context->y = posY;
                context->l = left;
                context->r = right;
                context->t_l = top_left;
                context->b_l = i++; // Don't test the same pixel again
                llist_push_front(&llist_context, context);
                posX = i;
                posY++;
                recurse = false;
                break;
              }
            }

            if (!recurse) {
              break;
            }
            if (llist_context.head_ptr == NULL) {
              break_out = true;
              break;
            }

            xylr_t* context = (xylr_t*)llist_pop_front(&llist_context);
            posX = context->x;
            posY = context->y;
            left = context->l;
            right = context->r;
            top_left = context->t_l;
            bot_left = context->b_l;
            llist_push_front(&llist_context_stack, context);
            blob_height++;
          } // END while_B

          if (break_out) {
            break;
          }
        } // END while_A

        if (blob_pixels > MIN_BLOB_PIX) {
          blob_t* blob_ptr = (blob_t*)llist_pop_front(&llist_blobs_stack);
          blob_ptr->timeTag = millis();

          //Serial.printf("\nDEBUG_CSLIDER:\tblobX:\t%f\tblobY:\t%f", (float)blob_cx / blob_pixels, (float)blob_cy / blob_pixels);

          blob_ptr->centroid.X = constrain(blob_cx / blob_pixels, X_MIN, X_MAX) - X_MIN ;
          blob_ptr->centroid.Y = constrain(blob_cy / blob_pixels, Y_MIN, Y_MAX) - Y_MIN;

          //Serial.printf("\nDEBUG_CSLIDER:\tblobX:\t%f\tblobY:\t%f", blob_ptr->centroid.X, blob_ptr->centroid.Y);

          blob_ptr->box.W = (blob_x2 - blob_x1);
          blob_ptr->box.H = blob_height;
          blob_ptr->centroid.Z = blob_depth - presets[THRESHOLD].val;

          llist_push_front(&llist_blobs_temp, blob_ptr);
        }
        posX = oldX;
        posY = oldY;
      }
    }
  }

  /////////////////////////////// PERSISTANT BLOB ID

  // Suppress DEAD blobs from the outputBlobs linked list
  while (1) {
    boolean deadFound = false;
    blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs);
    if (blob_ptr != NULL && blob_ptr->status == TO_REMOVE) {
      deadFound = true;
      blob_ptr = (blob_t*)llist_pop_front(&blobs);
      blob_ptr->status = FREE;
      llist_push_front(&llist_blobs_stack, blob_ptr);
      //Serial.printf("\nDEBUG_FIND_BLOBS / Blob: %p removed from **outputBlobs** linked list", (lnode_t*)blob_ptr);
    }
    if (!deadFound) {
      break;
    }
  }

  // NEW BLOBS MANAGMENT
  // Look for corresponding blobs into the **inputBlobs** and **outputBlobs** linked list
  for (blob_t* blobIn_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs_temp); blobIn_ptr != NULL; blobIn_ptr = (blob_t*)ITERATOR_NEXT(blobIn_ptr)) {
    float minDist = 255.0f;
    blob_t* nearestBlob = NULL;
    for (blob_t* blobOut = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blobOut != NULL; blobOut = (blob_t*)ITERATOR_NEXT(blobOut)) {
      float dist = sqrtf(pow(blobIn_ptr->centroid.X - blobOut->centroid.X, 2) + pow(blobIn_ptr->centroid.Y - blobOut->centroid.Y, 2));
#if DEBUG_FIND_BLOBS
      Serial.printf("\nDEBUG_FIND_BLOBS / Distance between input & output blobs positions: %f ", dist);
#endif
      if (dist < minDist) {
        minDist = dist;
        nearestBlob = blobOut;
      }
    }
    // If the distance between curent blob and last blob position is less than minDist:
    // Give the nearestBlob UID to the input blob.
    if (minDist < 4) {
#if DEBUG_FIND_BLOBS
      Serial.printf("\nDEBUG_FIND_BLOBS / Found corresponding blob: %p in the **outputBlobs** linked list", (lnode_t*)nearestBlob);
#endif
      blobIn_ptr->UID = nearestBlob->UID;
      blobIn_ptr->lastState = true;
      blobIn_ptr->state = true;
    }
    // Found a new blob! We nead to give it a UID
    else {
#if DEBUG_FIND_BLOBS
      Serial.print("\nDEBUG_FIND_BLOBS / Found new blob without ID");
#endif
      // Find the smallest missing UID in the outputBlobs linked list
      uint8_t minID = 0;
      while (1) {
        boolean isFree = true;
        for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
          if (blob_ptr->UID == minID) {
            isFree = false;
            minID++;
            break;
          }
        }
        if (isFree) {
          blobIn_ptr->UID = minID;
          blobIn_ptr->lastState = false;
          blobIn_ptr->state = true;
          break;
        }
      } // while_end / The blob have a new ID
    }
  }

  // DEAD BLOBS MANAGMENT
  // Look for dead blobs in the outputBlobs linked list
  // If found flag it TO_REMOVE
  while (1) {
    boolean allDone = true;
    blob_t* prevBlob_ptr = NULL;
    for (blob_t* blobOut_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blobOut_ptr != NULL; blobOut_ptr = (blob_t*)ITERATOR_NEXT(blobOut_ptr)) {
      boolean found = false;
      for (blob_t* blobIn_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs_temp); blobIn_ptr != NULL; blobIn_ptr = (blob_t*)ITERATOR_NEXT(blobIn_ptr)) {
        if (blobOut_ptr->UID == blobIn_ptr->UID) {
          found = true;
          break;
        }
      }
      if (!found) {
        allDone = false;
        llist_extract_node(&blobs, prevBlob_ptr, blobOut_ptr);
        blobOut_ptr->status = NOT_FOUND;
        //Serial.printf("\nDEBUG_FIND_BLOBS / Blob: %p in the **outputBlobs** linked list is NOT_FOUND", (lnode_t*)blobOut_ptr);
        if ((millis() - blobOut_ptr->timeTag) > DEBOUNCE_TIME) {
          blobOut_ptr->state = false;
          blobOut_ptr->status = TO_REMOVE;
          //Serial.printf("\nDEBUG_FIND_BLOBS / Blob: %p in the **outputBlobs** linked list taged TO_REMOVE", (lnode_t*)blobOut_ptr);
        }
        llist_push_front(&llist_blobs_temp, blobOut_ptr);
        break;
      }
      prevBlob_ptr = blobOut_ptr;
    }
    if (allDone) {
      break;
    }
  }

  llist_swap_llist(&blobs, &llist_blobs_temp);             // Swap inputBlobs with outputBlobs linked list
  llist_save_nodes(&llist_blobs_stack, &llist_blobs_temp); // Save/rescure all dead blobs Linked list nodes

#if VELOCITY
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {
      if (!blob_ptr->lastState) {
        blob_ptr->velocity.timeTag = millis();
        lastCoord[blob_ptr->UID].X = blob_ptr->centroid.X;
        lastCoord[blob_ptr->UID].Y = blob_ptr->centroid.Y;
        lastCoord[blob_ptr->UID].Z = blob_ptr->centroid.Z;
      }
      else {
        if (millis() - blob_ptr->velocity.timeTag > 150) {
          blob_ptr->velocity.timeTag = millis();
          float vx = fabs(blob_ptr->centroid.X - lastCoord[blob_ptr->UID].X);
          float vy = fabs(blob_ptr->centroid.Y - lastCoord[blob_ptr->UID].Y);
          blob_ptr->velocity.XY = sqrtf(vx * vx + vy * vy);
          blob_ptr->velocity.Z = blob_ptr->centroid.Z - lastCoord[blob_ptr->UID].Z;

          lastCoord[blob_ptr->UID].X = blob_ptr->centroid.X;
          lastCoord[blob_ptr->UID].Y = blob_ptr->centroid.Y;
          lastCoord[blob_ptr->UID].Z = blob_ptr->centroid.Z;
        };
      };
    };
  };
#endif
#if DEBUG_BITMAP
  for (uint8_t rowPos = 0; rowPos < NEW_ROWS; rowPos++) {
    uint8_t* rowPos_ptr = &bitmapArray[0] + rowPos * NEW_ROWS;
    for (int colPos = 0; colPos < NEW_COLS; colPos++) {
      Serial.printf("%d-", IMAGE_GET_PIXEL_FAST(rowPos_ptr, colPos);
    };
    Serial.printf("\n");
  }
  Serial.printf("\n");
#endif
#if DEBUG_BLOBS
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    Serial.printf("\nDEBUG_BLOBS:%d\tLS:%d\tS:%d\tX:%f\tY:%f\tZ:%d\tW:%d\tH:%d\tVXY:%f\tVZ:%f",
                  blob_ptr->UID,
                  blob_ptr->lastState,
                  blob_ptr->state,
                  blob_ptr->centroid.X,
                  blob_ptr->centroid.Y,
                  blob_ptr->centroid.Z,
                  blob_ptr->box.W,
                  blob_ptr->box.H,
                  blob_ptr->velocity.XY,
                  blob_ptr->velocity.Z
                 );
  }
#endif
};
