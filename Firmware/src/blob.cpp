/*
  FORKED FROM https://github.com/openmv/openmv/blob/master/lib/imlib/blob.c
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

#define LIFO_NODES          2048       // Set the maximum nodes number
#define X_STRIDE            4          // Speed up X scanning
#define Y_STRIDE            2          // Speed up Y scanning

uint8_t bitmap_array[NEW_FRAME] = {0}; // Store (64*64) binary values
xylr_t lifo_array[LIFO_NODES] = {0};   // Store lifo nodes
blob_t blob_array[MAX_BLOBS] = {0};    // Store blobs

llist_t llist_context_pool;            // Context nodes pool
llist_t llist_context;                 // Context nodes linked list
llist_t llist_blobs_pool;              // Blobs nodes pool
llist_t blobs_to_keep;                 // Swap buffer used during frame recycling
llist_t llist_blobs;                   // Blobs nodes linked list

void blob_setup(void) {
  llist_builder(&llist_context_pool, &lifo_array[0], LIFO_NODES, sizeof(lifo_array[0]));
  llist_builder(&llist_blobs_pool, &blob_array[0], MAX_BLOBS, sizeof(blob_array[0]));
  llist_raz(&llist_context);
  llist_raz(&blobs_to_keep);
  llist_raz(&llist_blobs);

  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs_pool); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
    blob_ptr->UID = 0;
    blob_ptr->status = FREE;
    blob_ptr->last_status = MISSING;
    blob_ptr->action.touch_ptr = NULL;
    blob_ptr->action.mapping_ptr = NULL;
  };
};

// Monotonically increasing UID — wraps at 128 to stay within MIDI SysEx 7-bit data range (0-127).
inline uint8_t set_id(void) {
  static uint8_t UID = 0;
  uint8_t id = UID;
  UID = (UID + 1) & 0x7F;
  return id;
};

/////////////////////////////// Scanline flood fill algorithm / SFF
/////////////////////////////// Connected-component labeling / CCL

void matrix_find_blobs(void) {

  blob_t* tmp_blob_ptr = NULL;

  // ---- Frame recycling ----
  // FREE blobs are released to the pool (their mapping slot is freed first).
  // All others carry their last status forward and start this frame as MISSING;
  // any blob the scanner finds again will be promoted back to PRESENT (or NEW).
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

  // ---- SFF seed scan ----
  // Strided outer loop (X_STRIDE, Y_STRIDE) for speed: each unvisited above-threshold
  // pixel seeds a full flood-fill that marks all connected pixels in the bitmap.
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

        // while_A: iterates over horizontal scanlines of the blob.
        // Each iteration processes one row: expands left/right, accumulates centroid,
        // then pushes neighbours above/below onto the context stack for while_B.
        while (1) { // while_A
          // posX can reach NEW_COLS when the push block sets posX = i++ with i == NEW_COLS-1.
          // Cap it so the mark loop and depth reads never go out of bounds.
          if (posX >= NEW_COLS) posX = NEW_COLS - 1;
          uint8_t left = posX;
          uint8_t right = posX;

          uint8_t* row_ptr_b = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY);
          uint8_t* bmp_row_ptr_b = &bitmap_array[0] + posY * NEW_COLS;

          // Expand the current run as far left and right as connected threshold pixels allow.
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

          // Mark visited and track peak depth (maximum sensor value = deepest press).
          for (int i = left; i <= right; i++) {
            IMAGE_SET_PIXEL_FAST(bmp_row_ptr_b, i, 1);
            blob_depth = MAX(blob_depth, IMAGE_GET_PIXEL_FAST(row_ptr_b, i));
          };

          // Weighted centroid accumulation: sum of column indices over the run.
          // sum = left + (left+1) + ... + right = (right*(right+1) - left*(left-1)) / 2
          float sum = ((right * (right + 1)) - (left * (left - 1))) / 2;

          uint8_t pixels = right - left + 1;
          blob_pixels += pixels;
          blob_cx += sum;
          blob_cy += posY * pixels;

          // Early exit: blob is already too large to be classified — bail out now
          // rather than exploring thousands more pixels and appearing to freeze.
          // Drain llist_context first so the next seed starts with a clean stack.
          if (blob_pixels > BLOB_MAX_PIX) {
            while (llist_context.head_ptr != NULL) {
              xylr_t* ctx = (xylr_t*)llist_pop_front(&llist_context);
              if (ctx) llist_push_front(&llist_context_pool, ctx);
            }
            break; // exits while_A; blob_pixels > BLOB_MAX_PIX so blob is dropped
          }

          uint8_t top_left = left;
          uint8_t bot_left = left;

          bool break_out = false;

          // while_B: iterative context stack replaces recursion.
          // Searches for unvisited connected pixels in the row above (top) and below (bot),
          // saves current state to the stack, then jumps to the neighbour row.
          while (1) { // while_B

            if (posY > 0) {
              row_ptr_b = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY - 1);
              bmp_row_ptr_b = &bitmap_array[0] + (posY - 1) * NEW_COLS;

              bool recurse = true;
              for (uint8_t i = top_left; i <= right; i++) {

                if ((!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_b, i))
                    && (PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_b, i), e256_ctr.levels[THRESHOLD].val))) {

                  // Save state and jump up one row to process the new seed.
                  xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_pool);
                  if (!context) { IMAGE_SET_PIXEL_FAST(bmp_row_ptr_b, i, 1); recurse = false; break; } // pool exhausted: mark to avoid re-finding
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
                };
              };
              if (!recurse) {
                break;
              };
            };

            if (posY < NEW_ROWS - 1) {
              row_ptr_b = COMPUTE_IMAGE_ROW_PTR(&interp_frame, posY + 1);
              bmp_row_ptr_b = &bitmap_array[0] + (posY + 1) * NEW_COLS;

              bool recurse = true;
              for (uint8_t i = bot_left; i <= right; i++) {

                if (!IMAGE_GET_PIXEL_FAST(bmp_row_ptr_b, i) &&
                    PIXEL_THRESHOLD(IMAGE_GET_PIXEL_FAST(row_ptr_b, i), e256_ctr.levels[THRESHOLD].val)) {

                  // Save state and jump down one row to process the new seed.
                  xylr_t* context = (xylr_t*)llist_pop_front(&llist_context_pool);
                  if (!context) { IMAGE_SET_PIXEL_FAST(bmp_row_ptr_b, i, 1); recurse = false; break; } // pool exhausted: mark to avoid re-finding
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
                };
              };

              if (!recurse) {
                break;
              };
            };
            // Stack empty: blob is fully explored.
            if (llist_context.head_ptr == NULL) {
              break_out = true;
              break;
            };

            // Pop and restore saved state to continue where we left off.
            xylr_t* context = (xylr_t*)llist_pop_front(&llist_context);
            if (!context) { break_out = true; break; }
            posX = context->x;
            posY = context->y;
            left = context->l;
            right = context->r;
            top_left = context->t_l;
            bot_left = context->b_l;
            llist_push_front(&llist_context_pool, context);
            blob_height++;
          }; // END while_B

          if (break_out) {
            break;
          };
        }; // END while_A

        // ---- Blob classification ----
        // Discard blobs outside the pixel-count range (noise or entire-palm touches).
        if (blob_pixels > BLOB_MIN_PIX && blob_pixels < BLOB_MAX_PIX) {

          // Stack-allocated temp for centroid comparison — never consumes the pool,
          // so existing blobs can always be re-found even when the pool is exhausted.
          blob_t match_blob;
          match_blob.centroid.x = (blob_cx / blob_pixels);
          match_blob.centroid.y = (blob_cy / blob_pixels);

          // Check if this blob matches one already tracked (proximity test).
          blob_t* blob_ptr = (blob_t*)llist_find_node(&llist_blobs, &match_blob, (llist_compare_func_t*)&is_blob_existing);

          // ---- Status state machine ----
          if (blob_ptr) {
            // Known blob found again — promote it out of MISSING.
            if (blob_ptr->last_status == MISSING) {
              // Blob reappeared within the grace window: treat as continuous contact.
              if ((millis() - blob_ptr->life_time_stamp) < BLOB_MISSING_TIME) {
                blob_ptr->status = PRESENT;
                #if defined(USB_MIDI_SERIAL) && defined(DEBUG_BLOBS_STATUS)
                Serial.printf("\nD_STATUS: %s\tLAST_STATUS: %s", get_blob_status_name(blob_ptr->status), get_blob_status_name(blob_ptr->last_status));
                #endif
              }
            }
            else {
              if (blob_ptr->last_status == PRESENT) {
                blob_ptr->status = PRESENT;
                #if defined(USB_MIDI_SERIAL) && defined(DEBUG_BLOBS_STATUS)
                Serial.printf("\nF_STATUS: %s\tLAST_STATUS: %s", get_blob_status_name(blob_ptr->status), get_blob_status_name(blob_ptr->last_status));
                #endif
              }
              else {
                if (blob_ptr->last_status == RELEASED) {
                  // Finger lifted then touched again quickly: reuse the same blob node
                  // so the mapping association (touch slot, note) is preserved.
                  blob_ptr->status = NEW;
                  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_BLOBS_STATUS)
                  Serial.printf("\nG_STATUS: %s\tLAST_STATUS: %s", get_blob_status_name(blob_ptr->status), get_blob_status_name(blob_ptr->last_status));
                  #endif
                }
                else {
                  blob_ptr->status = PRESENT;
                  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_BLOBS_STATUS)
                  Serial.printf("\nH_STATUS: %s\tLAST_STATUS: %s", get_blob_status_name(blob_ptr->status), get_blob_status_name(blob_ptr->last_status));
                  #endif
                }
              }
            }

            blob_ptr->centroid.x = match_blob.centroid.x;
            blob_ptr->centroid.y = match_blob.centroid.y;
            blob_ptr->centroid.z = min(blob_depth, 127);
            blob_ptr->box.w = (blob_x2 - blob_x1);
            blob_ptr->box.h = blob_height;

            blob_ptr->life_time_stamp = millis();
          }
          else if (blob_count < MAX_BLOBS) {
            // New blob — consume a pool node only when adding a truly new blob.
            blob_t* new_blob_ptr = (blob_t*)llist_pop_front(&llist_blobs_pool);
            if (new_blob_ptr) {
              blob_count++;
              new_blob_ptr->UID = set_id();
              new_blob_ptr->last_status = FREE;
              new_blob_ptr->status = NEW;

              new_blob_ptr->centroid.x = match_blob.centroid.x;
              new_blob_ptr->centroid.y = match_blob.centroid.y;
              new_blob_ptr->centroid.z = min(blob_depth, 127);
              new_blob_ptr->box.w = (blob_x2 - blob_x1);
              new_blob_ptr->box.h = blob_height;

              new_blob_ptr->life_time_stamp = millis();

              llist_push_back(&llist_blobs, new_blob_ptr);
            }
          }
        }
        posX = oldX;
        posY = oldY;
      }
    }
  }

  // ---- Lost blob timeout ----
  // Blobs still MISSING after the scan have not been seen this frame.
  // Two-stage expiry: MISSING → RELEASED (grace window) → FREE (fully expired).
  // RELEASED gives the MIDI layer one last chance to send NoteOff before the slot is recycled.
  blob_t* lost_blob_ptr = NULL;

  while ((lost_blob_ptr = (blob_t*)llist_pop_front(&llist_blobs)) != NULL) {
    if (lost_blob_ptr->status == MISSING) {
      uint32_t blob_life_time = (millis() - lost_blob_ptr->life_time_stamp);
      if (blob_life_time < BLOB_MISSING_TIME) {
        // Still within grace window — keep as MISSING.
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

  // ---- Velocity & attack tracking (compiled only when BLOB_VELOCITY is defined) ----
  //
  // Two quantities are computed per blob each frame:
  //   velocity.xy  — smoothed lateral speed (Euclidean, units/s)
  //   velocity.z   — smoothed vertical speed (signed: >0 pressing, <0 releasing, units/s)
  //
  // Both are filtered with an EMA (α = VELOCITY_EMA_ALPHA) to suppress sensor noise.
  //
  // Attack detection avoids a fixed-time NoteOn delay:
  //   1. On first contact (NEW), attack_z is zeroed and note_on_z_pending is raised.
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
  #if defined(BLOB_VELOCITY)
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);
    uint32_t now = millis();

    if (blob_ptr->status == NEW) {
      // First detection: arm the attack window and freeze references.
      blob_ptr->velocity.time_stamp_z     = now;
      blob_ptr->velocity.time_stamp_xy    = now;
      blob_ptr->velocity.born_at          = now;
      blob_ptr->velocity.xy_last_x        = blob_ptr->centroid.x;
      blob_ptr->velocity.xy_last_y        = blob_ptr->centroid.y;
      blob_ptr->last_centroid             = blob_ptr->centroid;
      blob_ptr->velocity.xy               = 0.0f;
      blob_ptr->velocity.z                = 0.0f;
      blob_ptr->velocity.attack_z         = 0.0f;
      blob_ptr->velocity.attack_done      = false;
      blob_ptr->action.note_on_z_pending  = false;
      blob_ptr->action.note_on_xy_pending = false;
    }
    else if (blob_ptr->status == PRESENT) {

      // XY velocity: throttled to VELOCITY_MIN_INTERVAL_MS to suppress centroid jitter.
      // At 600fps a 1-pixel jitter gives ~590 units/s; averaging over 10ms reduces this.
      uint32_t dt_xy = now - blob_ptr->velocity.time_stamp_xy;
      if (dt_xy >= VELOCITY_MIN_INTERVAL_MS) {
        float dt_s = dt_xy * 0.001f;
        float vx = blob_ptr->centroid.x - blob_ptr->velocity.xy_last_x;
        float vy = blob_ptr->centroid.y - blob_ptr->velocity.xy_last_y;
        float raw_vel_xy = sqrtf(vx * vx + vy * vy) / dt_s;
        blob_ptr->velocity.xy = VELOCITY_EMA_ALPHA * raw_vel_xy + (1.0f - VELOCITY_EMA_ALPHA) * blob_ptr->velocity.xy;
        blob_ptr->velocity.xy_last_x = blob_ptr->centroid.x;
        blob_ptr->velocity.xy_last_y = blob_ptr->centroid.y;
        blob_ptr->velocity.time_stamp_xy = now;
      }

      // Z velocity and attack: throttled to VELOCITY_MIN_INTERVAL_MS for stability.
      uint32_t dt = now - blob_ptr->velocity.time_stamp_z;
      if (dt >= VELOCITY_MIN_INTERVAL_MS) {
        float dt_s = dt * 0.001f;
        float raw_vel_z = (float)((int16_t)blob_ptr->centroid.z - (int16_t)blob_ptr->last_centroid.z) / dt_s;
        blob_ptr->velocity.z = VELOCITY_EMA_ALPHA * raw_vel_z + (1.0f - VELOCITY_EMA_ALPHA) * blob_ptr->velocity.z;
        blob_ptr->last_centroid = blob_ptr->centroid;
        blob_ptr->velocity.time_stamp_z = now;

        if (!blob_ptr->velocity.attack_done) {
          float abs_vz = fabsf(blob_ptr->velocity.z);
          uint32_t age = now - blob_ptr->velocity.born_at;
          if (abs_vz > blob_ptr->velocity.attack_z) {
            blob_ptr->velocity.attack_z = abs_vz;
          } else if (age >= VELOCITY_ATTACK_MIN_MS && abs_vz < blob_ptr->velocity.attack_z * VELOCITY_ATTACK_DROP) {
            blob_ptr->velocity.attack_done = true;
          }
          if (age >= VELOCITY_ATTACK_MAX_MS) blob_ptr->velocity.attack_done = true;
        }
      }
    }
    else if (blob_ptr->status == RELEASED) {
      // Keep attack_z intact — the MIDI layer may still need it for NoteOff velocity.
      blob_ptr->velocity.xy = 0.0f; // redondant !?
      blob_ptr->velocity.z  = 0.0f; // redondant !?
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
    Serial.printf("\nDEBUG_BLOBS:%d\tS:%d\tX:%f\tY:%f\tZ:%d\tW:%d\tH:%d\tVXY:%f\tVZ:%f",
      blob_ptr->UID,
      blob_ptr->status,
      blob_ptr->centroid.x,
      blob_ptr->centroid.y,
      blob_ptr->centroid.z,
      blob_ptr->box.w,
      blob_ptr->box.h,
      blob_ptr->velocity.xy,
      blob_ptr->velocity.z
    );
  };
  #endif

  #if defined(USB_MIDI_SERIAL) && defined(DEBUG_FIND_BLOBS)
  if ((lnode_t*)llist_blobs.head_ptr != NULL) {
    Serial.printf("\n___________DEBUG_FIND_BLOBS / END_OF_FRAME");
  }
  #endif
};

// Two blobs match if their centroids are within BLOB_LAST_DIST pixels — used to
// correlate detections across frames without requiring exact positional overlap.
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
