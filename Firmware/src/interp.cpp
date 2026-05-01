/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// Catmull-Rom bicubic interpolation (RAW 16×16 → interp_frame 64×64)
//
// Replaces bilinear: bilinear is only C0-continuous (value-continuous), which
// creates a gradient jump at every raw-cell boundary. This makes blobs look
// blocky and introduces ~1-pixel centroid jitter at cell crossings.
//
// Catmull-Rom is C1-continuous: value AND first derivative are smooth across
// cell boundaries. The result is a pressure map that looks organic and gives
// the blob tracker sub-pixel centroid accuracy.
//
// Trade-off: uses a 4×4 neighborhood per source cell instead of 2×2, so ~4×
// more float multiplications. Still well within Teensy 4.0 FPU budget.

#include "interp.h"
#include "config.h"
#include "scan.h"

static_assert(SCALE_X == SCALE_Y, "Catmull-Rom impl requires SCALE_X == SCALE_Y");

uint8_t interp_threshold = 5;
uint8_t interp_frame_array[NEW_FRAME] = {0};
image_t interp_frame;

// Catmull-Rom 1D weights for each of the SCALE_X sub-positions (t = k / SCALE_X).
// catmull_w[k][d] is the weight of neighbor d in {0=i-1, 1=i, 2=i+1, 3=i+2}.
// The same table is used for both axes since SCALE_X == SCALE_Y.
static float catmull_w[SCALE_X][4];

void interp_setup(void) {
  interp_frame.data_ptr = &interp_frame_array[0];
  interp_frame.num_cols = NEW_COLS;
  interp_frame.num_rows = NEW_ROWS;

  // Standard Catmull-Rom basis (α = 0.5):
  //   w0(t) = -0.5t³ +  t²  - 0.5t         (neighbor i-1)
  //   w1(t) =  1.5t³ - 2.5t² + 1            (neighbor i  ) ← interpolates through sample
  //   w2(t) = -1.5t³ + 2t²  + 0.5t          (neighbor i+1) ← interpolates through sample
  //   w3(t) =  0.5t³ - 0.5t²                (neighbor i+2)
  // At t=0: [0,1,0,0]; at t=1: [0,0,1,0] — passes through every raw sample.
  for (uint8_t k = 0; k < SCALE_X; k++) {
    float t  = (float)k / SCALE_X;
    float t2 = t * t;
    float t3 = t2 * t;
    catmull_w[k][0] = -0.5f * t3 + t2        - 0.5f * t;
    catmull_w[k][1] =  1.5f * t3 - 2.5f * t2 + 1.0f;
    catmull_w[k][2] = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
    catmull_w[k][3] =  0.5f * t3 - 0.5f * t2;
  }
}

inline void update_interp_threshold(level_t* levels_ptr) {
  if (levels_ptr->leds.update) {
    levels_ptr->leds.update = false;
    interp_threshold = constrain(levels_ptr->val - 4, 0, levels_ptr->max_val);
  }
}

// Catmull-Rom bicubic upscaling.
// Windowing: source cells at or below threshold are skipped (output stays 0).
// Edge clamping: neighbors outside [0, RAW_ROWS/COLS - 1] are clamped to the border.
// Output clamped to [0, 255]: Catmull-Rom can produce slight overshoot (<3% on smooth data).
void matrix_interp(void) {
  update_interp_threshold(&e256_ctr.levels[THRESHOLD]);
  memset(&interp_frame_array[0], 0, SIZEOF_FRAME);

  for (uint8_t row_pos = 0; row_pos < RAW_ROWS; row_pos++) {
    uint8_t* raw_row_ptr = COMPUTE_IMAGE_ROW_PTR(&raw_frame, row_pos);

    for (uint8_t col_pos = 0; col_pos < RAW_COLS; col_pos++) {
      if (IMAGE_GET_PIXEL_FAST(raw_row_ptr, col_pos) <= interp_threshold) continue;

      // Cache the 4×4 raw neighborhood (edge-clamped) to avoid repeated pointer arithmetic.
      uint8_t nb[4][4];
      for (int8_t dy = -1; dy <= 2; dy++) {
        uint8_t r = (uint8_t)constrain((int)row_pos + dy, 0, RAW_ROWS - 1);
        uint8_t* nb_row_ptr = COMPUTE_IMAGE_ROW_PTR(&raw_frame, r);
        for (int8_t dx = -1; dx <= 2; dx++) {
          uint8_t c = (uint8_t)constrain((int)col_pos + dx, 0, RAW_COLS - 1);
          nb[dy + 1][dx + 1] = IMAGE_GET_PIXEL_FAST(nb_row_ptr, c);
        }
      }

      // Write SCALE_Y × SCALE_X output pixels for this source cell.
      for (uint8_t ky = 0; ky < SCALE_Y; ky++) {
        uint16_t out_row = (uint16_t)row_pos * SCALE_Y + ky;
        if (out_row >= NEW_ROWS) break;

        for (uint8_t kx = 0; kx < SCALE_X; kx++) {
          uint16_t out_col = (uint16_t)col_pos * SCALE_X + kx;
          if (out_col >= NEW_COLS) break;

          // Separable 2D Catmull-Rom: outer loop Y, inner loop X.
          float val = 0.0f;
          for (uint8_t dy = 0; dy < 4; dy++) {
            float wy = catmull_w[ky][dy];
            for (uint8_t dx = 0; dx < 4; dx++) {
              val += wy * catmull_w[kx][dx] * (float)nb[dy][dx];
            }
          }
          interp_frame_array[out_row * NEW_COLS + out_col] =
            (uint8_t)constrain((int)lroundf(val), 0, 255);
        }
      }
    }
  }

#if defined(USB_MIDI_SERIAL) && defined(DEBUG_INTERP)
  for (uint8_t row_pos = 0; row_pos < NEW_ROWS; row_pos++) {
    uint8_t* rowPos_ptr = &interp_frame_array[0] + row_pos * NEW_ROWS;
    for (int col_pos = 0; col_pos < NEW_COLS; col_pos++) {
      Serial.printf("%d-", IMAGE_GET_PIXEL_FAST(rowPos_ptr, col_pos));
    };
    Serial.printf("\n");
  };
  Serial.printf("\n\n");
#endif
}
