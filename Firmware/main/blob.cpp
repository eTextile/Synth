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

#define MAX_BLOBS               16        // [1:64] Set how many blobs can be tracked at the same time
#define LIFO_MAX_NODES          255       // Set the maximum nodes number
#define X_STRIDE                3         // 
#define Y_STRIDE                3         // 
#define MIN_BLOB_PIX            8         // Set the minimum blob pixels
#define MAX_BLOB_PIX            4095      // Set the maximum blob pixels
#define DEBOUNCE_TIME_BLOB      15        // TODO

uint8_t bitmapFrame[NEW_FRAME] = {0};     // 1D Array to store (64*64) binary values
xylr_t lifoArray[LIFO_MAX_NODES] = {0};   // 1D Array to store lifo nodes
blob_t blobArray[MAX_BLOBS] = {0};        // 1D Array to store blobs

velocity_t blobVelocity[MAX_SYNTH] = {0}; // 1D Array to store XY & Z blobs velocity
polar_t polarCoord[MAX_SYNTH] = {0};      // 1D Array of struct polar_t to store blobs polar coordinates

llist_t llist_context_stack;    // Free nodes stack
llist_t llist_context;          // Used nodes
llist_t llist_blobs_stack;      // Free nodes stack
llist_t llist_blobs;            // Intermediate blobs linked list

void blob_llist_init(llist_t* llist_ptr, blob_t* nodesArray_ptr) {
  llist_raz(llist_ptr);
  for (int i = 0; i < MAX_BLOBS; i++) {
    llist_push_front(llist_ptr, &nodesArray_ptr[i]);
  }
}

void lifo_llist_init(llist_t* llist_ptr, xylr_t* nodesArray_ptr) {
  llist_raz(llist_ptr);
  for (int i = 0; i < LIFO_MAX_NODES; i++) {
    llist_push_front(llist_ptr, &nodesArray_ptr[i]);
  }
}

void bitmap_clear() {
  memset(&bitmapFrame[0], 0, NEW_FRAME * sizeof(uint8_t));
}

void BLOB_SETUP(llist_t* outputBlobs_ptr) {
  lifo_llist_init(&llist_context_stack, &lifoArray[0]); // Add X nodes to the llist_context_stack
  blob_llist_init(&llist_blobs_stack, &blobArray[0]); // Add X nodes to the llist_blobs_stack linked list
  llist_raz(&llist_context);
  llist_raz(&llist_blobs);
  llist_raz(outputBlobs_ptr);
}

static int sum_m_to_n(int m, int n) {
  return ((n * (n + 1)) - (m * (m - 1))) / 2;
}

float distance(blob_t* blobA, blob_t* blobB) {
  float sum = 0.0f;
  sum += (blobA->centroid.X - blobB->centroid.X) * (blobA->centroid.X - blobB->centroid.X);
  sum += (blobA->centroid.Y - blobB->centroid.Y) * (blobA->centroid.Y - blobB->centroid.Y);
  return sqrtf(sum);
}

void find_blobs(
  uint8_t   zThreshold,
  image_t*  inputFrame_ptr,
  llist_t*  outputBlobs_ptr
) {

  /////////////////////////////// Scanline flood fill algorithm / SFF
  /////////////////////////////// Connected-component labeling / CCL

  bitmap_clear();
  static uint16_t lifoNodes = 0;
  //Serial.println(lifoNodes);

  for (uint8_t posY = 0; posY < NEW_ROWS; posY += Y_STRIDE) {

    uint8_t* row_ptr_A = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY);
    uint8_t* bmp_row_ptr_A = COMPUTE_BINARY_IMAGE_ROW_PTR(&bitmapFrame[0], posY);

    for (uint8_t posX = (posY % X_STRIDE); posX < NEW_COLS; posX += X_STRIDE) {
      if (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_A, posX)
          && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_A, posX), zThreshold)) {

        uint8_t oldX = posX;
        uint8_t oldY = posY;

        uint8_t blob_x1 = posX;
        uint8_t blob_x2 = posX;

        uint8_t blob_height = 0;
        uint8_t blob_depth = 0;

        uint16_t blob_pixels = 0;
        uint16_t blob_cx = 0;
        uint16_t blob_cy = 0;

        while (1) { // while_A
          uint8_t left = posX;
          uint8_t right = posX;

          uint8_t* row_ptr_B = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY);
          uint8_t* bmp_row_ptr_B = COMPUTE_BINARY_IMAGE_ROW_PTR (&bitmapFrame[0], posY);

          while ((left > 0)
                 && (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, left - 1))
                 && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, left - 1), zThreshold)) {
            left--;
          }

          while (right < (NEW_COLS - 1)
                 && (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, right + 1))
                 && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, right + 1), zThreshold)) {
            right++;
          }

          blob_x1 = MIN(blob_x1, left);
          blob_x2 = MAX(blob_x2, right);

          for (uint8_t i = left; i <= right; i++) {
            IMAGE_SET_BINARY_PIXEL_FAST(bmp_row_ptr_B, i);
            blob_depth = MAX(blob_depth, IMAGE_GET_PIXEL_FAST(row_ptr_B, i));
          }

          int sum = sum_m_to_n(left, right);
          uint8_t pixels = right - left + 1;
          blob_pixels += pixels;
          blob_cx += sum;
          blob_cy += posY * pixels;

          uint8_t top_left = left;
          uint8_t bot_left = left;

          boolean break_out = false;

          while (1) { // while_B

            if (lifoNodes < LIFO_MAX_NODES) {

              if (posY > 0) {

                row_ptr_B = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY - 1);
                bmp_row_ptr_B = COMPUTE_BINARY_IMAGE_ROW_PTR(&bitmapFrame[0], posY - 1);

                boolean recurse = false;
                for (uint8_t i = top_left; i <= right; i++) {

                  if ((!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, i))
                      && (PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), zThreshold))) {

                    xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_stack);
                    //Serial.printf("\nDEBUG_LIFO / A / llist_context_stack / llist_pop_front: %p", (lnode_t*)context);

                    context->x = posX;
                    context->y = posY;
                    context->l = left;
                    context->r = right;
                    context->t_l = i++; // Don't test the same pixel again
                    context->b_l = bot_left;

                    llist_push_front(&llist_context, context);
                    lifoNodes++;
                    //Serial.printf("\nDEBUG_LIFO / A / llist_context / llist_push_front: %p", (lnode_t*)context);

                    posX = i;
                    posY--;
                    recurse = true;
                    break;
                  }
                }
                if (recurse) {
                  break;
                }
              }

              if (posY < (NEW_ROWS - 1)) {

                row_ptr_B = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY + 1);
                bmp_row_ptr_B = COMPUTE_BINARY_IMAGE_ROW_PTR(&bitmapFrame[0], posY + 1);

                boolean recurse = false;
                for (uint8_t i = bot_left; i <= right; i++) {

                  if (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, i)
                      && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), zThreshold)) {

                    xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_stack);
                    //Serial.printf("\nDEBUG_LIFO / B / llist_context_stack / llist_pop_front: %p", (lnode_t*)context);

                    context->x = posX;
                    context->y = posY;
                    context->l = left;
                    context->r = right;
                    context->t_l = top_left;
                    context->b_l = i++; // Don't test the same pixel again

                    llist_push_front(&llist_context, context);
                    lifoNodes++;
                    //Serial.printf("\nDEBUG_LIFO / B / llist_context / llist_push_front: %p", (lnode_t*)context);

                    posX = i;
                    posY++;
                    recurse = true;
                    break;
                  }
                }
                if (recurse) {
                  break;
                }
              }
            }
            if (lifoNodes <= 0) {
              break_out = true;
              break;
            }

            xylr_t* context = (xylr_t*)llist_pop_front(&llist_context);
            //Serial.printf("\nDEBUG_LIFO / C / llist_context / llist_pop_front: %p", (lnode_t*)context);

            posX = context->x;
            posY = context->y;
            left = context->l;
            right = context->r;
            top_left = context->t_l;
            bot_left = context->b_l;

            llist_push_front(&llist_context_stack, context);
            lifoNodes--;
            //Serial.printf("\nDEBUG_LIFO / C / llist_context_stack / llist_push_front: %p", (lnode_t*)context);

            blob_height++;

          } // END while_B

          if (break_out) {
            break;
          }
        } // END while_A

        if (blob_pixels > MIN_BLOB_PIX && blob_pixels < MAX_BLOB_PIX) {

          blob_t* blob = (blob_t*)llist_pop_front(&llist_blobs_stack);

          //blob->timeTag = millis(); // TODO
          blob->centroid.X = blob_cx / (float)blob_pixels;
          blob->centroid.Y = blob_cy / (float)blob_pixels;
          blob->box.W = (blob_x2 - blob_x1);
          blob->box.H = blob_height;
          blob->box.D = blob_depth - zThreshold; // NEED TO ADD A LIMIT?
          /*
            Serial.printf("\nDEBUG_SFF / blob_X:%f \tblob_Y:%f \tblob_W:%d \tblob_H:%d \tblob_D:%d",
                        blob->centroid.X,
                        blob->centroid.Y,
                        blob->box.W,
                        blob->box.H,
                        blob->box.D
                       );
          */
          llist_push_front(&llist_blobs, blob);
          //Serial.printf("\n DEBUG_SFF / Blob: %p added to the **blobs** linked list", (lnode_t*)blob);
        }
        posX = oldX;
        posY = oldY;
      }
    }
  }

  /////////////////////////////// PERSISTANT BLOB ID

  // Suppress DEAD blobs from the outputBlobs linked list
  while (1) {
    boolean found = false;
    blob_t* prevBlob_ptr = NULL;

    for (blob_t* blob = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blob != NULL; blob = (blob_t*)ITERATOR_NEXT(blob)) {
      if (blob->status == TO_REMOVE) {
        found = true;
        blob->status = FREE;
        blob->lastState = false;
        llist_extract_node(outputBlobs_ptr, prevBlob_ptr, blob);
        llist_push_front(&llist_blobs_stack, blob);
#if DEBUG_BLOBS_ID
        Serial.printf("\nDEBUG_BLOBS_ID / Blob: %p removed from **outputBlobs** linked list", (lnode_t*)blob);
#endif
        break;
      }
      prevBlob_ptr = blob;
    }
    if (!found) {
      break;
    }
  }

  // NEW BLOBS MANAGMENT
  // Look for corresponding blobs into the **inputBlobs** and **outputBlobs** linked list
  for (blob_t* blobIn = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blobIn != NULL; blobIn = (blob_t*)ITERATOR_NEXT(blobIn)) {
    float dist = 0;
    float minDist = 255.0f;
    blob_t* nearestBlob = NULL;

    for (blob_t* blobOut = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blobOut != NULL; blobOut = (blob_t*)ITERATOR_NEXT(blobOut)) {
      dist = distance(blobIn, blobOut);
#if DEBUG_BLOBS_ID
      Serial.printf("\nDEBUG_BLOBS_ID / Distance between input & output blobs positions: %f ", dist);
#endif
      if (dist < minDist) {
        minDist = dist;
        nearestBlob = blobOut;
      }
    }
    // If the distance between curent blob and last blob position is less than minDist:
    // Copy the ID of the nearestBlob found in outputBlobs linked list and give it to the curent input blob.
    if (minDist < 2.0f) {
#if DEBUG_BLOBS_ID
      Serial.printf("\nDEBUG_BLOBS_ID / Found corresponding blob: %p in the **outputBlobs** linked list", (lnode_t*)nearestBlob);
#endif
      blobIn->UID = nearestBlob->UID;
      blobIn->lastState = true;
      blobIn->state = true;
    }
    // Found a new blob! We nead to give it an ID
    else {
#if DEBUG_BLOBS_ID
      Serial.print("\n DEBUG_BLOBS_ID / Found new blob without ID");
#endif
      // Find the smallest missing UID in the outputBlobs linked list
      uint8_t minID = 0;
      while (1) {
        boolean isFree = true;
        for (blob_t* blob = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blob != NULL; blob = (blob_t*)ITERATOR_NEXT(blob)) {
          if (blob->UID == minID) {
            isFree = false;
            minID++;
            break;
          }
        }
        if (isFree) {
          blobIn->UID = minID;
          blobIn->lastState = false;
          blobIn->state = true;
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
    for (blob_t* blobOut = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blobOut != NULL; blobOut = (blob_t*)ITERATOR_NEXT(blobOut)) {
      boolean found = false;
      for (blob_t* blobIn = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blobIn != NULL; blobIn = (blob_t*)ITERATOR_NEXT(blobIn)) {
        if (blobOut->UID == blobIn->UID) {
          found = true;
          break;
        }
      }
      if (!found) {
        allDone = false;
        blobOut->status = TO_REMOVE;
        blobOut->lastState = true;
        blobOut->state = false;
        llist_extract_node(outputBlobs_ptr, prevBlob_ptr, blobOut);
        llist_push_front(&llist_blobs, blobOut);
#if DEBUG_BLOBS_ID
        Serial.printf("\nDEBUG_BLOBS_ID / Blob: %p in the **outputBlobs** linked list taged TO_REMOVE", (lnode_t*)blobOut);
#endif
        break;
      }
      prevBlob_ptr = blobOut;
    }
    if (allDone) {
      break;
    }
  }

  llist_swap_llist(outputBlobs_ptr, &llist_blobs);     // Swap inputBlobs with outputBlobs linked list
  llist_save_nodes(&llist_blobs_stack, &llist_blobs);  // Save all inputBlobs Linked list nodes

#if DEBUG_BLOBS_ID
  Serial.printf("\nDEBUG_BLOBS_ID / END OFF BLOB FONCTION");
#endif
}

void getBlobsVelocity(llist_t* blobs_ptr) {
  float vx = 0;
  float vy = 0;

  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {
      vx = blob_ptr->centroid.X - blobVelocity[blob_ptr->UID].lastPos.X;
      vy = blob_ptr->centroid.Y - blobVelocity[blob_ptr->UID].lastPos.Y;

      blobVelocity[blob_ptr->UID].vxy = sqrt(vx * vx + vy * vy); //pow(vx, 2) + pow(vy, 2)
      blobVelocity[blob_ptr->UID].vz = blob_ptr->box.D - blobVelocity[blob_ptr->UID].lastVz;

      blobVelocity[blob_ptr->UID].lastPos.X = blob_ptr->centroid.X;
      blobVelocity[blob_ptr->UID].lastPos.Y = blob_ptr->centroid.Y;
      blobVelocity[blob_ptr->UID].lastVz = blob_ptr->box.D;

#if DEBUG_MAPPING
      Serial.printf("\nDEBUG_VELOCITY : vxy:%f\tvz:%f",
                    blobVelocity[blob_ptr->UID].vxy,
                    blobVelocity[blob_ptr->UID].vz
                   );
#endif
    }
  }
}

void getPolarCoordinates(llist_t* blobs_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    float posX = blob_ptr->centroid.X - CENTER_X;
    float posY = blob_ptr->centroid.Y - CENTER_Y;
    if (posX == 0 && posY == 0 ) {
      polarCoord[blob_ptr->UID].r = 0;
      polarCoord[blob_ptr->UID].phi = 0;
    }
    else {
      polarCoord[blob_ptr->UID].r = sqrt(posX * posX + posY * posY);
      if (posX == 0 && 0 < posY) {
        polarCoord[blob_ptr->UID].phi = PI / 2;
      } else if (posX == 0 && posY < 0) {
        polarCoord[blob_ptr->UID].phi = PI * 3 / 2;
      } else if (posX < 0) {
        polarCoord[blob_ptr->UID].phi = atan(posY / posX) + PI;
      } else if (posY < 0) {
        polarCoord[blob_ptr->UID].phi = atan(posY / posX) + 2 * PI;
      } else {
        polarCoord[blob_ptr->UID].phi = atan(posY / posX);
      }
    }
#if DEBUG_MAPPING
    Serial.printf("\nDEBUG_POLAR : R: % f\tPHY: % f", polarCoord[blob_ptr->UID].r, polarCoord[blob_ptr->UID].phi);
#endif
  }
}

#if DEBUG_BITMAP
void print_bitmap(void) {
  for (uint8_t posY = 0; posY < NEW_ROWS; posY++) {
    uint8_t* row_ptr = COMPUTE_BINARY_IMAGE_ROW_PTR(&bitmapFrame[0], posY);
    for (uint8_t posX = 0; posX < NEW_COLS; posX++) {
      IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, posX) == 0 ? Serial.printf(".") : Serial.printf("o");
    }
    Serial.printf("\n");
  }
  Serial.printf("\n");
}
#endif

#if DEBUG_BLOBS
void print_blobs(llist_t* llist_ptr) {
  uint8_t index = 0;
  for (blob_t* blob = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob != NULL; blob = (blob_t*)ITERATOR_NEXT(blob)) {
    Serial.printf("\nINDEX:%d\tID:%d\tS:%d\tLS:%d\tX:%f\tY:%f\tW:%d\tH:%d\tD:%d\t",
                  index, blob->UID,
                  blob->state,
                  blob->lastState,
                  blob->centroid.X,
                  blob->centroid.Y,
                  blob->box.W,
                  blob->box.H,
                  blob->box.D
                 );
    index++;
  }
}
#endif
