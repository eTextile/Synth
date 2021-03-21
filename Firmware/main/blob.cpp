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
    llist_push_back(list_ptr, &nodesArray[i]);
  }
}

void lifo_llist_init(llist_t *list_ptr, xylr_t* nodesArray, uint8_t max_nodes) {
  for (int i = 0; i < max_nodes; i++) {
    llist_push_back(list_ptr, &nodesArray[i]);
  }
}

void node_raz(blob_t* node) {
  node->UID = -1;
  node->timeTag = millis();
  node->state = false;
  node->lastState = false;
  node->status = FREE;
  //node->centroid.X = 0.0f;
  //node->centroid.Y = 0.0f;
  //node->box.W = 0;
  //node->box.H = 0;
  //node->box.D = 0;
  //node->pixels = 0;
}

void bitmap_clear(image_t* bitmap_ptr) {
  memset(bitmap_ptr->pData, 0, NEW_FRAME * sizeof(uint8_t));
}

float distance(blob_t* blobA, blob_t* blobB) {
  float sum = 0.0f;
  sum += (blobA->centroid.X - blobB->centroid.X) * (blobA->centroid.X - blobB->centroid.X);
  sum += (blobA->centroid.Y - blobB->centroid.Y) * (blobA->centroid.Y - blobB->centroid.Y);
  return sqrtf(sum);
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
  Serial.println("DONE_SETUP");
}

static int sum_m_to_n(int m, int n) {
  return ((n * (n + 1)) - (m * (m - 1))) / 2;
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
  Serial.println("DEBUG_2");

  for (uint8_t posY = 0, yy = inputFrame_ptr->numRows, y_max = yy - 1; posY < yy; posY += Y_STRIDE) {
    Serial.println("DEBUG_1");

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

        Serial.println("DEBUG_0");

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

            Serial.println("DEBUG_A");
            if (lifoNodes < LIFO_MAX_NODES) {

              if (posY > 0) {

                row_ptr_B = COMPUTE_IMAGE_ROW_PTR(inputFrame_ptr, posY - 1);
                bmp_row_ptr_B = COMPUTE_BINARY_IMAGE_ROW_PTR(bitmap_ptr, posY - 1);

                boolean recurse = false;
                for (uint8_t i = top_left; i <= right; i++) {

                  if ((!IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr_B, i))
                      && (PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_B, i), zThreshold))) {

                    Serial.println("DEBUG_B");
                    xylr_t* context = (xylr_t*)llist_pop_front(lifo_stack_ptr);
                    Serial.printf("\n DEBUG_LIFO / A / lifo_stack_ptr / llist_pop_front: %p", (lnode_t*)context);

                    context->x = posX;
                    context->y = posY;
                    context->l = left;
                    context->r = right;
                    context->t_l = i++; // Don't test the same pixel again
                    context->b_l = bot_left;

                    Serial.printf("\n DEBUG_LIFO / A / lifo_ptr / llist_push_front: %p", (lnode_t*)context);
                    llist_push_front(lifo_ptr, context);
                    lifoNodes++;

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
                    Serial.printf("\n DEBUG_LIFO / B / lifo_stack_ptr / llist_pop_front: %p", (lnode_t*)context);

                    context->x = posX;
                    context->y = posY;
                    context->l = left;
                    context->r = right;
                    context->t_l = top_left;
                    context->b_l = i++; // Don't test the same pixel again

                    llist_push_front(lifo_ptr, context);
                    lifoNodes++;
                    Serial.printf("\n DEBUG_LIFO / B / lifo_ptr / llist_push_front: %p", (lnode_t*)context);

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
            if (lifoNodes < 0) {
              break_out = true;
              break;
            }

            xylr_t* context = (xylr_t*)llist_pop_front(lifo_ptr);
            Serial.printf("\n DEBUG_LIFO / C / lifo_ptr / llist_pop_front: %p", (lnode_t*)context);

            posX = context->x;
            posY = context->y;
            left = context->l;
            right = context->r;
            top_left = context->t_l;
            bot_left = context->b_l;

            llist_push_front(lifo_stack_ptr, context);
            lifoNodes--;
            Serial.printf("\n DEBUG_LIFO / C / lifo_stack_ptr / llist_push_front: %p", (lnode_t*)context);

            blob_height++;

          } // END while_B

          if (break_out) {
            break;
          }
        } // END while_A

        if (blob_pixels > MIN_BLOB_PIX && blob_pixels < MAX_BLOB_PIX) {

          blob_t* blob = (blob_t*)llist_pop_front(blobs_stack_ptr);
          node_raz(blob);

          blob->centroid.X = blob_cx / (float)blob_pixels;
          blob->centroid.Y = blob_cy / (float)blob_pixels;
          blob->box.W = (blob_x2 - blob_x1);
          blob->box.H = blob_height;
          blob->box.D = blob_depth - zThreshold;

          Serial.printf("\n DEBUG_SFF / blob_X:%f \tblob_Y:%f \tblob_W:%d \tblob_H:%d \tblob_D:%d",
                        blob->centroid.X,
                        blob->centroid.Y,
                        blob->box.W,
                        blob->box.H,
                        blob->box.D
                       );

          llist_push_back(inputBlobs_ptr, blob);
          Serial.printf("\n DEBUG_SFF / Blob: %p added to the **blobs** linked list", (lnode_t*)blob);
        }
        posX = oldX;
        posY = oldY;
      }
    }
  }
  /////////////////////////////// PERSISTANT BLOB ID

#if DEBUG_BLOBS_ID
  //Serial.printf("\n DEBUG_BLOBS_ID / **inputBlobs** linked list index: %d", inputBlobs_ptr->index);
  //Serial.printf("\n DEBUG_BLOBS_ID / **blobs_stack** linked list index: %d", blobs_stack_ptr->index);
  //Serial.printf("\n DEBUG_BLOBS_ID / **outputBlobs** linked list index: %d", outputBlobs_ptr->index);
#endif

  // Suppress blobs from the outputBlobs linked list
  while (1) {
    boolean found = false;
    for (blob_t* blob = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blob != NULL; blob = (blob_t*)ITERATOR_NEXT(blob)) {
      if (blob->status == TO_REMOVE) {
        found = true;
        blob_t* blobExtract = (blob_t*)llist_extract_node(outputBlobs_ptr, blob);
        llist_push_back(blobs_stack_ptr, blobExtract);
#if DEBUG_BLOBS_ID
        Serial.printf("\n DEBUG_BLOBS_ID / Blob: %p removed from **outputBlobs** linked list", (lnode_t*)blobExtract);
        Serial.printf("\n DEBUG_BLOBS_ID / Blob: %p saved to **blobsStack** linked list", (lnode_t*)blobExtract);
#endif
        break;
      }
    }
    if (!found) {
      break;
    }
  }

  // Look for the nearest blob between curent blob position (inputBlobs linked list) and last blob position (outputBlobs linked list)
  for (blob_t* blobA = (blob_t*)ITERATOR_START_FROM_HEAD(inputBlobs_ptr); blobA != NULL; blobA = (blob_t*)ITERATOR_NEXT(blobA)) {
    float minDist = 255.0f;
    blob_t* nearestBlob = NULL;

#if DEBUG_BLOBS_ID
    Serial.printf("\n DEBUG_BLOBS_ID / Is input blob: %p have a coresponding blob in **outputBlobs**", (lnode_t*)blobA);
#endif
    for (blob_t* blobB = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blobB != NULL; blobB = (blob_t*)ITERATOR_NEXT(blobB)) {

      float dist = distance(blobA, blobB);

#if DEBUG_BLOBS_ID
      Serial.printf("\n DEBUG_BLOBS_ID / Distance between input & output blobs positions: %f ", dist);
#endif
      if (dist < minDist) {
        minDist = dist;
        nearestBlob = blobB;
      }
    }
    // If the distance between curent blob and last blob position is less than minDist:
    // Copy the ID of the nearestBlob found in outputBlobs linked list and give it to the curent input blob.
    // Set the curent input blob state to TO_UPDATE.
    if (minDist < 3.0f) {
#if DEBUG_BLOBS_ID
      Serial.printf("\n DEBUG_BLOBS_ID / Found corresponding blob: %p in the **outputBlobs** linked list", (lnode_t*)nearestBlob);
#endif
      blobA->UID = nearestBlob->UID;
      blobA->status = TO_UPDATE;
    }
    // Found a new blob! We nead to give it an ID
    else {
#if DEBUG_BLOBS_ID
      Serial.print("\n DEBUG_BLOBS_ID / Found new blob without ID");
#endif
      // Find the smallest missing UID in the sorted linked list
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
          blobA->UID = minID;
          blobA->status = TO_ADD;
          break;
        }
      } // while_end / The blob have a new ID
    }
  }

  // Update outputBlobs linked list with inputs blobs that have ben flaged TO_UPDATE.
  // If a blob in the outputBlobs linked do not have corresponding blob in the input blobs linked list (inputBlobs_ptr), flag it to TO_REMOVE.
  for (blob_t* blobA = (blob_t*)ITERATOR_START_FROM_HEAD(outputBlobs_ptr); blobA != NULL; blobA = (blob_t*)ITERATOR_NEXT(blobA)) {
    boolean found = false;

    for (blob_t* blobB = (blob_t*)ITERATOR_START_FROM_HEAD(inputBlobs_ptr); blobB != NULL; blobB = (blob_t*)ITERATOR_NEXT(blobB)) {
      if (blobB->status == TO_UPDATE && blobB->UID == blobA->UID) {
        found = true;

        //llist_swap_nodes(outputBlobs_ptr, blobA, inputBlobs_ptr, blobB); // TODO
        blob_t* supprBlob = (blob_t*)llist_extract_node(outputBlobs_ptr, blobA);
        llist_push_back(blobs_stack_ptr, supprBlob);
        blob_t* addBlob = (blob_t*)llist_extract_node(inputBlobs_ptr, blobB);
        llist_push_back(outputBlobs_ptr, addBlob);

#if DEBUG_BLOBS_ID
        Serial.printf("\n DEBUG_BLOBS_ID / Copy blob: %p (inputBlobs linked list) to the blob: %p (outputBlobs linked list)", (lnode_t*)blobB, (lnode_t*)blobA);
#endif
        break;
      }
    }
    if (!found && millis() - blobA->timeTag > DEBOUNCE_TIME_BLOB) {
      blobA->status = TO_REMOVE;
      blobA->state = false;
    }
  }

  // Add the new blobs to the outputBlobs linked list
  while (1) {
    boolean allDone = true;
    for (blob_t* blob = (blob_t*)ITERATOR_START_FROM_HEAD(inputBlobs_ptr); blob != NULL; blob = (blob_t*)ITERATOR_NEXT(blob)) {
      if (blob->status == TO_ADD) {
        allDone = false;

        llist_push_back(outputBlobs_ptr, blob); // FIXME
#if DEBUG_BLOBS_ID
        Serial.printf("\n DEBUG_BLOBS_ID / Blob: %p added to **outputBlobs** linked list", (lnode_t*)blob);
#endif
      }
      break;
    }
    if (allDone) {
      break;
    }
  }

  // Clear the input blobs Linked list
  llist_save_nodes(blobs_stack_ptr, inputBlobs_ptr); // TODO : can we live without it ?

#if DEBUG_BLOBS_ID
  Serial.println("\n DEBUG_BLOBS_ID / END OFF BLOB FONCTION");
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
void print_blobs(llist_t* src_ptr) {
  for (blob_t* blob = (blob_t*)ITERATOR_START_FROM_HEAD(src_ptr); blob != NULL; blob = (blob_t*)ITERATOR_NEXT(blob)) {
    Serial.printf("\nINDEX:%d\tID:%d\tS:%d\tX:%f\tY:%f\tW:%d\tH:%d\tD:%d\t", blob->UID, blob->state, blob->centroid.X, blob->centroid.Y, blob->box.W, blob->box.H, blob->box.D);
  }
}
#endif
