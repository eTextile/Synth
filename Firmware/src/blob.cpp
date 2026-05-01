/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/src/omv/imlib/blob.c
    - This file is part of the OpenMV project.
    - Copyright (c) 2013- Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
    - This work is licensed under the MIT license, see the file LICENSE for details.

  Added custom blob détection algorithm to keep track of the blobs ID's
    - This patch is part of the eTextile-Synthesizer project - https://synth.eTextile.org
    - Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
    - This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "interp.h"
#include "blob.h"
#include "median.h"
#include "mapping.h"

#define LIFO_NODES          1024       // Set the maximum nodes number
#define X_STRIDE            4          // Speed up X scanning
#define Y_STRIDE            2          // Speed up Y scanning

uint8_t bitmap_array[NEW_FRAME] = {0}; // Store (64*64) binary values
xylr_t lifo_array[LIFO_NODES] = {0};   // Store lifo nodes
blob_t blob_array[MAX_BLOBS] = {0};    // Store blobs

llist_t llist_context_pool;            // Context nodes pool
llist_t llist_context;                 // Context nodes linked list
llist_t llist_blobs_pool;              // Blobs nodes pool
llist_t blobs_to_keep;                 // Tmp 
llist_t llist_blobs;                   // Blobs nodes linked list
//llist_t llist_previous_blobs;

void blob_setup(void) {
  llist_builder(&llist_context_pool, &lifo_array[0], LIFO_NODES, sizeof(lifo_array[0])); // Add X nodes to the llist_context_pool
  llist_builder(&llist_blobs_pool, &blob_array[0], MAX_BLOBS, sizeof(blob_array[0])); // Add X nodes to the llist_blobs_pool
  llist_raz(&llist_context);
  llist_raz(&blobs_to_keep);
  llist_raz(&llist_blobs);

  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs_pool); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
    blob_ptr->UID = 0;
    blob_ptr->status = MISSING;
    blob_ptr->last_status = FREE;
    blob_ptr->action.touch_ptr = NULL;
    blob_ptr->action.mapping_ptr = NULL;
  };
};

inline uint8_t set_id(void) {
  static uint8_t UID = 0;
  return UID++;
};

/////////////////////////////// Scanline flood fill algorithm / SFF
/////////////////////////////// Connected-component labeling / CCL

void matrix_find_blobs(void) {

  blob_t* tmp_blob_ptr = NULL;

  while ((tmp_blob_ptr = (blob_t*)llist_pop_front(&llist_blobs)) != NULL) {
    if (tmp_blob_ptr->status == FREE) {
      common_t* blob_mapping_ptr = (common_t*)tmp_blob_ptr->action.mapping_ptr;
      if (blob_mapping_ptr) blob_mapping_ptr->blob_dispose_func_ptr(blob_mapping_ptr, tmp_blob_ptr);
      llist_push_back(&llist_blobs_pool, tmp_blob_ptr);
    }
    else {
      tmp_blob_ptr->last_status = tmp_blob_ptr->status;
      tmp_blob_ptr->status = MISSING;
      llist_push_back(&blobs_to_keep, tmp_blob_ptr);
    }
  }
  llist_swap_llist(&llist_blobs, &blobs_to_keep);

  memset(&bitmap_array[0], 0, SIZEOF_FRAME);

  uint8_t blob_count = 0;
  
  for (uint8_t posY = 0; posY < NEW_ROWS; posY += Y_STRIDE) {

    uint8_t* row_ptr_a = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY);
    uint8_t* bmp_row_ptr_A = &bitmap_array[0] + posY * NEW_COLS;

    for (uint8_t posX = (posY % X_STRIDE); posX < NEW_COLS; posX += X_STRIDE) {
      if (!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_A, posX) &&
          PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_a, posX), e256_ctr.levels[THRESHOLD].val)) {

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

          uint8_t* row_ptr_b = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY);
          uint8_t* bmp_row_ptr_b = &bitmap_array[0] + posY * NEW_COLS;

          while ((left > 0) &&
                 !IMAGE_GET_PIXEL_FAST(bmp_row_ptr_b, left - 1) &&
                 PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_b, left - 1), e256_ctr.levels[THRESHOLD].val)) {
            left--;
          };
          while (right < (NEW_COLS - 1) &&
                 !IMAGE_GET_PIXEL_FAST(bmp_row_ptr_b, right + 1) &&
                 PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_b, right + 1), e256_ctr.levels[THRESHOLD].val)) {
            right++;
          };

          blob_x1 = MIN(blob_x1, left);
          blob_x2 = MAX(blob_x2, right);

          for (int i = left; i <= right; i++) {
            IMAGE_SET_PIXEL_FAST(bmp_row_ptr_b, i, 1);
            blob_depth = MAX(blob_depth, IMAGE_GET_PIXEL_FAST(row_ptr_b, i));
          };

          float sum = ((right * (right + 1)) - (left * (left - 1))) / 2;

          uint8_t pixels = right - left + 1;
          blob_pixels += pixels;
          blob_cx += sum;
          blob_cy += posY * pixels;

          uint8_t top_left = left;
          uint8_t bot_left = left;

          bool break_out = false;

          while (1) { // while_B

            if (posY > 0) {
              row_ptr_b = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY - 1);
              bmp_row_ptr_b = &bitmap_array[0] + (posY - 1) * NEW_COLS;

              bool recurse = true;
              for (uint8_t i = top_left; i <= right; i++) {

                if ((!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_b, i))
                    && (PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_b, i), e256_ctr.levels[THRESHOLD].val))) {

                  xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_pool);
                  context->x = posX;
                  context->y = posY;
                  context->l = left;
                  context->r = right;
                  context->t_l = i++; // Don't test the same pixel again
                  context->b_l = bot_left;
                  if (context) llist_push_front(&llist_context, context);
                  posX = i;
                  posY--;
                  recurse = false;
                  break;
                };
              };
              if (!recurse) {
                break;
              };
            };

            row_ptr_b = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY + 1);
            bmp_row_ptr_b = &bitmap_array[0] + (posY + 1) * NEW_COLS;

            bool recurse = true;
            for (uint8_t i = bot_left; i <= right; i++) {

              if (!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_b, i) &&
                  PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_b, i), e256_ctr.levels[THRESHOLD].val)) {
                
                xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_pool);
                context->x = posX;
                context->y = posY;
                context->l = left;
                context->r = right;
                context->t_l = top_left;
                context->b_l = i++; // Don't test the same pixel again
                if (context) llist_push_front(&llist_context, context);
                posX = i;
                posY++;
                recurse = false;
                break;
              };
            };

            if (!recurse) {
              break;
            };
            if (llist_context.head_ptr == NULL) {
              break_out = true;
              break;
            };

            xylr_t* context = (xylr_t*)llist_pop_front(&llist_context);
            posX = context->x;
            posY = context->y;
            left = context->l;
            right = context->r;
            top_left = context->t_l;
            bot_left = context->b_l;
            if (context) llist_push_front(&llist_context_pool, context);
            blob_height++;
          }; // END while_B

          if (break_out) {
            break;
          };
        }; // END while_A

        if (blob_pixels > BLOB_MIN_PIX && blob_pixels < BLOB_MAX_PIX && blob_count < MAX_BLOBS) {
          blob_t* undefined_blob_ptr = (blob_t*)llist_pop_front(&llist_blobs_pool);
          if (undefined_blob_ptr) {

          undefined_blob_ptr->centroid.x = (blob_cx / blob_pixels);
          undefined_blob_ptr->centroid.y = (blob_cy / blob_pixels);

          // Is blob in the list ?
          blob_t* blob_ptr = (blob_t*)llist_find_node(&llist_blobs, undefined_blob_ptr, (llist_compare_func_t*)&is_blob_existing);
          
          // We need to update the blob status
          if (blob_ptr) {
            if (blob_ptr->last_status == MISSING) {
              if ((millis() - blob_ptr->life_time_stamp) < BLOB_MISSING_TIME) {
                blob_ptr->status = PRESENT;
                //Serial.printf("\nD_STATUS: %s\tLAST_STATUS: %s", get_blob_status_name(blob_ptr->status), get_blob_status_name(blob_ptr->last_status));
              }
            }
            else {
              if (blob_ptr->last_status == PRESENT) {
                blob_ptr->status = PRESENT; // MISSING?
                //Serial.printf("\nF_STATUS: %s\tLAST_STATUS: %s", get_blob_status_name(blob_ptr->status), get_blob_status_name(blob_ptr->last_status));
              }
              else {
                if (blob_ptr->last_status == RELEASED) {
                  blob_ptr->status = NEW; // it still have it's mapping associate
                  //Serial.printf("\nG_STATUS: %s\tLAST_STATUS: %s", get_blob_status_name(blob_ptr->status), get_blob_status_name(blob_ptr->last_status));
                }
                else {
                  blob_ptr->status = PRESENT;
                  //Serial.printf("\nH_STATUS: %s\tLAST_STATUS: %s", get_blob_status_name(blob_ptr->status), get_blob_status_name(blob_ptr->last_status));
                }
              }
            }

            blob_ptr->centroid.x = undefined_blob_ptr->centroid.x;
            blob_ptr->centroid.y = undefined_blob_ptr->centroid.y;
            blob_ptr->centroid.z = min(blob_depth, 127);
            blob_ptr->box.w = (blob_x2 - blob_x1);
            blob_ptr->box.h = blob_height;

            //Serial.printf("\nBLOB_FOUND_POS: X:%f Y:%f Z:%d", blob_ptr->centroid.x, blob_ptr->centroid.y, blob_ptr->centroid.z);

            blob_ptr->life_time_stamp = millis(); // RAZ

            llist_push_back(&llist_blobs_pool, undefined_blob_ptr);
          }
          else {
            blob_count++;
            undefined_blob_ptr->UID = set_id();
            undefined_blob_ptr->last_status = FREE;
            undefined_blob_ptr->status = NEW;

            //Serial.printf("\nI_STATUS: %s\tLAST_STATUS: %s", get_blob_status_name(undefined_blob_ptr->status), get_blob_status_name(undefined_blob_ptr->last_status));

            //undefined_blob_ptr->action.touch_ptr = NULL;
            //undefined_blob_ptr->action.mapping_ptr = NULL;

            undefined_blob_ptr->centroid.z = min(blob_depth, 127);
            undefined_blob_ptr->box.w = (blob_x2 - blob_x1);
            undefined_blob_ptr->box.h = blob_height;

            undefined_blob_ptr->life_time_stamp = millis();

            llist_push_back(&llist_blobs, undefined_blob_ptr);
          }
          } // if (undefined_blob_ptr)
        }
        posX = oldX;
        posY = oldY;
      }
    }
  }

  // At last we test the blobs that have disappeared but which are still in the list
  blob_t* lost_blob_ptr = NULL;

  while ((lost_blob_ptr = (blob_t*)llist_pop_front(&llist_blobs)) != NULL) {
    if (lost_blob_ptr->status == MISSING) {
      uint32_t blob_life_time = (millis() - lost_blob_ptr->life_time_stamp);
      if (blob_life_time < BLOB_MISSING_TIME) {
        //lost_blob_ptr->status = MISSING; // NOT NEAD!
      }
      else {
        if (blob_life_time < BLOB_RELEASE_TIME) {
          lost_blob_ptr->status = RELEASED;
        }
        else {
          lost_blob_ptr->status = FREE;
        }
      }
    }
    llist_push_back(&blobs_to_keep, lost_blob_ptr);
  }
  llist_swap_llist(&llist_blobs, &blobs_to_keep);

  #if defined(RUNING_MEDIAN)
  runing_median();
  #endif

  // ---- Velocity & attack tracking (compiled only when VELOCITY is defined) ----
  //
  // Two quantities are computed per blob each frame:
  //   velocity.xy  — smoothed lateral speed (Euclidean, units/s)
  //   velocity.z   — smoothed vertical speed (signed: >0 pressing, <0 releasing, units/s)
  //
  // Both are filtered with an EMA (α = VELOCITY_EMA_ALPHA) to suppress sensor noise.
  //
  // Attack detection avoids a fixed-time NoteOn delay:
  //   1. On first contact (NEW), attack_z is zeroed and note_on_pending is raised.
  //   2. While pressing, attack_z tracks the running peak of |velocity.z|.
  //   3. attack_done is set as soon as |velocity.z| drops below
  //      VELOCITY_ATTACK_DROP × peak  AND  at least VELOCITY_ATTACK_MIN_MS have elapsed.
  //      A hard deadline (VELOCITY_ATTACK_MAX_MS) fires attack_done if the drop never occurs.
  //   4. The MIDI layer reads attack_done + attack_z to send the deferred NoteOn
  //      with a velocity proportional to the captured impact peak.
  //
  // Why peak-drop rather than a fixed window?
  //   A fixed window adds a constant latency on every note. With peak-drop, a sharp
  //   percussive tap closes the window in ~5-15 ms; a slow press may take longer but
  //   the perceptible latency is lower because the sound starts with the finger movement.
  #if defined(VELOCITY)
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
    uint32_t now = millis();

    if (blob_ptr->status == NEW) {
      // First detection: arm the attack window and freeze last_centroid as reference.
      blob_ptr->velocity.time_stamp = now;
      blob_ptr->velocity.born_at = now;
      blob_ptr->last_centroid = blob_ptr->centroid;
      blob_ptr->velocity.xy = 0.0f;
      blob_ptr->velocity.z = 0.0f;
      blob_ptr->velocity.attack_z = 0.0f;
      blob_ptr->velocity.attack_done = false;
      blob_ptr->action.note_on_pending = false; // mapping _start sets it to true only for NoteOn mode
    }
    else if (blob_ptr->status == PRESENT) {
      uint32_t dt = now - blob_ptr->velocity.time_stamp;
      if (dt >= VELOCITY_MIN_INTERVAL_MS) {
        float dt_s = dt * 0.001f;

        // Raw instantaneous velocities (units/s).
        float vx = blob_ptr->centroid.x - blob_ptr->last_centroid.x;
        float vy = blob_ptr->centroid.y - blob_ptr->last_centroid.y;
        float raw_vel_xy = sqrtf(vx * vx + vy * vy) / dt_s;
        float raw_vel_z  = (float)((int16_t)blob_ptr->centroid.z - (int16_t)blob_ptr->last_centroid.z) / dt_s;

        // EMA smoothing — reduces frame-to-frame sensor noise without a ring buffer.
        blob_ptr->velocity.xy = VELOCITY_EMA_ALPHA * raw_vel_xy + (1.0f - VELOCITY_EMA_ALPHA) * blob_ptr->velocity.xy;
        blob_ptr->velocity.z  = VELOCITY_EMA_ALPHA * raw_vel_z  + (1.0f - VELOCITY_EMA_ALPHA) * blob_ptr->velocity.z;

        blob_ptr->last_centroid = blob_ptr->centroid;
        blob_ptr->velocity.time_stamp = now;

        if (!blob_ptr->velocity.attack_done) {
          float abs_vz = fabsf(blob_ptr->velocity.z);
          uint32_t age = now - blob_ptr->velocity.born_at;

          if (abs_vz > blob_ptr->velocity.attack_z) {
            blob_ptr->velocity.attack_z = abs_vz; // still rising: update peak
          } else if (age >= VELOCITY_ATTACK_MIN_MS && abs_vz < blob_ptr->velocity.attack_z * VELOCITY_ATTACK_DROP) {
            blob_ptr->velocity.attack_done = true; // past the peak: impact is over
          }
          if (age >= VELOCITY_ATTACK_MAX_MS) blob_ptr->velocity.attack_done = true; // hard deadline
        }
      }
    }
    else if (blob_ptr->status == RELEASED) {
      // Keep attack_z intact — the MIDI layer may still need it for NoteOff velocity.
      blob_ptr->velocity.xy = 0.0f;
      blob_ptr->velocity.z  = 0.0f;
    }
  };
  #endif

  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_BITMAP)
  for (uint8_t row_pos = 0; row_pos < NEW_ROWS; row_pos++) {
    uint8_t* row_pos_ptr = &bitmap_array[0] + row_pos * NEW_ROWS;
    for (int col_pos = 0; col_pos < NEW_COLS; col_pos++) {
      Serial.printf("%d-", IMAGE_GET_PIXEL_FAST(row_pos_ptr, col_pos));
    };
    Serial.printf("\n");
  };
  Serial.printf("\n");
  #endif

  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_BLOBS)
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
    //Serial.printf("\nDEBUG_BLOBS:%d\tS:%d\tX:%f\tY:%f\tZ:%d\tW:%d\tH:%d\tVXY:%f\tVZ:%f",
    Serial.printf("\nDEBUG_BLOBS:%d\tS:%d\tX:%f\tY:%f\tZ:%d\tW:%d\tH:%d",
      blob_ptr->UID,
      blob_ptr->status,
      blob_ptr->centroid.x,
      blob_ptr->centroid.y,
      blob_ptr->centroid.z,
      blob_ptr->box.w,
      blob_ptr->box.h
      //blob_ptr->velocity.xy,
      //blob_ptr->velocity.z
    );
  };
  #endif

  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FIND_BLOBS)
  if ((lnode_t*)llist_blobs.head_ptr != NULL) {
    Serial.printf("\n___________DEBUG_FIND_BLOBS / END_OF_FRAME");
  }
  #endif
};

bool is_blob_existing(blob_t* blob_ptr, blob_t* undefined_blob_ptr) {
  float dist = sqrtf(pow(blob_ptr->centroid.x - undefined_blob_ptr->centroid.x, 2) + pow(blob_ptr->centroid.y - undefined_blob_ptr->centroid.y, 2));
  if (dist < BLOB_LAST_DIST) {
    return true;
  }
  return false;
};

const char* get_blob_status_name(status_code_t code) {
  const char* char_code = NULL;
  switch (code) {
    case FREE: char_code = "FREE"; break;
    case NEW: char_code = "NEW"; break;
    case PRESENT: char_code = "PRESENT"; break;
    case MISSING: char_code = "MISSING"; break;
    case RELEASED: char_code = "RELEASED"; break;
  }
  return char_code;
};