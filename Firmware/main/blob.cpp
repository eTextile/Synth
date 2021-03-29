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

void blob_llist_init(llist_t *list_ptr, blob_t* nodesArray, uint8_t max_nodes) {
  for (int i = 0; i < max_nodes; i++) {
    llist_push_front(list_ptr, &nodesArray[i]);
  }
}

void lifo_llist_init(llist_t *list_ptr, xylr_t* nodesArray, uint8_t max_nodes) {
  for (int i = 0; i < max_nodes; i++) {
    llist_push_front(list_ptr, &nodesArray[i]);
  }
}

void bitmap_clear(image_t* bitmap_ptr) {
  memset(bitmap_ptr->pData, 0, NEW_FRAME * sizeof(uint8_t));
}

void SETUP_BLOB(
  uint8_t* bitmapArray_ptr,
  image_t* bitmap_ptr,
  llist_t* lifo_ptr,
  llist_t* lifo_stack_ptr,
  xylr_t*  lifoArray_ptr,
  llist_t* blobs_ptr,
  llist_t* blobs_stack_ptr,
  blob_t*  blobArray_ptr,
  llist_t* outputBlobs_ptr
) {

  // Init bitmap (struct image_t)
  bitmap_ptr->numCols = NEW_COLS;           //
  bitmap_ptr->numRows = NEW_ROWS;           //
  bitmap_ptr->pData = &bitmapArray_ptr[0];  //

  // Init lifo linked list
  llist_raz(lifo_stack_ptr);
  lifo_llist_init(lifo_stack_ptr, &lifoArray_ptr[0], (uint8_t)LIFO_MAX_NODES); // Add X nodes to the lifo_stack
  llist_raz(lifo_ptr);

  // Init blobs linked list
  llist_raz(blobs_stack_ptr);
  blob_llist_init(blobs_stack_ptr, &blobArray_ptr[0], (uint8_t)MAX_BLOBS); // Add X nodes to the blobs_stack linked list
  llist_raz(blobs_ptr);

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
  image_t*  bitmap_ptr,
  llist_t*  lifo_stack_ptr,
  llist_t*  lifo_ptr,
  llist_t*  blobs_stack_ptr,
  llist_t*  inputBlobs_ptr,
  llist_t*  outputBlobs_ptr
) {

  /////////////////////////////// Scanline flood fill algorithm / SFF
  /////////////////////////////// Connected-component labeling / CCL

  bitmap_clear(bitmap_ptr);
  static uint16_t lifoNodes = 0;
  //Serial.println(lifoNodes);

  for (uint8_t posY = 0, yy = inputFrame_ptr->numRows, y_max = yy - 1; posY < yy; posY += Y_STRIDE) {

    uint8_t* row_ptr_A = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY);
    uint8_t* bmp_row_ptr_A = COMPUTE_BINARY_IMAGE_ROW_PTR(bitmap_ptr, posY);

    for (uint8_t posX = (posY % X_STRIDE), xx = inputFrame_ptr->numCols, x_max = xx - 1; posX < xx; posX += X_STRIDE) {
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

          uint8_t* row_ptr_B = COMPUTE_IMAGE_ROW_PTR (inputFrame_ptr, posY);
          uint8_t* bmp_row_ptr_B = COMPUTE_BINARY_IMAGE_ROW_PTR (bitmap_ptr, posY);

          while ((left > 0)
                 && (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, left - 1))
                 && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, left - 1), zThreshold)) {
            left--;
          }

          while (right < (inputFrame_ptr->numCols - 1)
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
                bmp_row_ptr_B = COMPUTE_BINARY_IMAGE_ROW_PTR(bitmap_ptr, posY - 1);

                boolean recurse = false;
                for (uint8_t i = top_left; i <= right; i++) {

                  if ((!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, i))
                      && (PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), zThreshold))) {

                    xylr_t* context = (xylr_t*)llist_pop_front(lifo_stack_ptr);
                    //Serial.printf("\nDEBUG_LIFO / A / lifo_stack_ptr / llist_pop_front: %p", (lnode_t*)context);

                    context->x = posX;
                    context->y = posY;
                    context->l = left;
                    context->r = right;
                    context->t_l = i++; // Don't test the same pixel again
                    context->b_l = bot_left;

                    llist_push_front(lifo_ptr, context);
                    lifoNodes++;
                    //Serial.printf("\nDEBUG_LIFO / A / lifo_ptr / llist_push_front: %p", (lnode_t*)context);

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

              if (posY < (inputFrame_ptr->numRows - 1)) {

                row_ptr_B = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY + 1);
                bmp_row_ptr_B = COMPUTE_BINARY_IMAGE_ROW_PTR(bitmap_ptr, posY + 1);

                boolean recurse = false;
                for (uint8_t i = bot_left; i <= right; i++) {

                  if (!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, i)
                      && PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), zThreshold)) {

                    xylr_t* context = (xylr_t*)llist_pop_front(lifo_stack_ptr);
                    //Serial.printf("\nDEBUG_LIFO / B / lifo_stack_ptr / llist_pop_front: %p", (lnode_t*)context);

                    context->x = posX;
                    context->y = posY;
                    context->l = left;
                    context->r = right;
                    context->t_l = top_left;
                    context->b_l = i++; // Don't test the same pixel again

                    llist_push_front(lifo_ptr, context);
                    lifoNodes++;
                    //Serial.printf("\nDEBUG_LIFO / B / lifo_ptr / llist_push_front: %p", (lnode_t*)context);

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

            xylr_t* context = (xylr_t*)llist_pop_front(lifo_ptr);
            //Serial.printf("\nDEBUG_LIFO / C / lifo_ptr / llist_pop_front: %p", (lnode_t*)context);

            posX = context->x;
            posY = context->y;
            left = context->l;
            right = context->r;
            top_left = context->t_l;
            bot_left = context->b_l;

            llist_push_front(lifo_stack_ptr, context);
            lifoNodes--;
            //Serial.printf("\nDEBUG_LIFO / C / lifo_stack_ptr / llist_push_front: %p", (lnode_t*)context);

            blob_height++;

          } // END while_B

          if (break_out) {
            break;
          }
        } // END while_A

        if (blob_pixels > MIN_BLOB_PIX && blob_pixels < MAX_BLOB_PIX) {

          blob_t* blob = (blob_t*)llist_pop_front(blobs_stack_ptr);

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
          llist_push_front(inputBlobs_ptr, blob);
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
        llist_push_front(blobs_stack_ptr, blob);
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
  for (blob_t* blobIn = (blob_t*)ITERATOR_START_FROM_HEAD(inputBlobs_ptr); blobIn != NULL; blobIn = (blob_t*)ITERATOR_NEXT(blobIn)) {
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
    // Set the curent input blob state to TO_UPDATE.
    if (minDist < 2.0f) {
#if DEBUG_BLOBS_ID
      Serial.printf("\nDEBUG_BLOBS_ID / Found corresponding blob: %p in the **outputBlobs** linked list", (lnode_t*)nearestBlob);
#endif
      blobIn->UID = nearestBlob->UID;
      //blobIn->status = TO_UPDATE;
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
          //blobIn->status = TO_ADD;
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
      for (blob_t* blobIn = (blob_t*)ITERATOR_START_FROM_HEAD(inputBlobs_ptr); blobIn != NULL; blobIn = (blob_t*)ITERATOR_NEXT(blobIn)) {
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
        llist_push_front(inputBlobs_ptr, blobOut);
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

  llist_swap_llist(outputBlobs_ptr, inputBlobs_ptr);  // Swap inputBlobs with outputBlobs linked list
  llist_save_nodes(blobs_stack_ptr, inputBlobs_ptr);  // Save all inputBlobs Linked list nodes

#if DEBUG_BLOBS_ID
  Serial.printf("\nDEBUG_BLOBS_ID / END OFF BLOB FONCTION");
#endif
}

#if DEBUG_BITMAP
void print_bitmap(image_t* bitmap_ptr) {
  for (uint8_t posY = 0; posY < bitmap_ptr->numRows; posY++) {
    uint8_t* row_ptr = COMPUTE_BINARY_IMAGE_ROW_PTR(bitmap_ptr, posY);
    for (uint8_t posX = 0; posX < bitmap_ptr->numCols; posX++) {
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
    Serial.printf("\nINDEX:%d\tID:%d\tS:%d\tX:%f\tY:%f\tW:%d\tH:%d\tD:%d\t", index, blob->UID, blob->state, blob->centroid.X, blob->centroid.Y, blob->box.W, blob->box.H, blob->box.D);
    index++;
  }
}
#endif
