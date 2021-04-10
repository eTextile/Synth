/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "interp.h"

float* coef_A[SCALE_X * SCALE_Y] = {0};
float* coef_B[SCALE_X * SCALE_Y] = {0};
float* coef_C[SCALE_X * SCALE_Y] = {0};
float* coef_D[SCALE_X * SCALE_Y] = {0};

/*
    Bilinear interpolation
    Pre-compute the four coefficient values for all interpolated output matrix positions
*/

void INTERP_SETUP(
  image_t* inputFrame_ptr,
  uint8_t* inputArray_ptr,
  image_t* outputFrame_ptr,
  uint8_t* outputArray_ptr,
  interp_t* interp_ptr
) {

  // image_t* inputFrame init config
  inputFrame_ptr->numCols = RAW_COLS;            // Setup -> RAW_COLS
  inputFrame_ptr->numRows = RAW_ROWS;            // Setup -> RAW_ROWS
  inputFrame_ptr->pData = &inputArray_ptr[0];    // Setup -> uint8_t frameArray[RAW_FRAME] (16x16)

  // image_t* outputFrame_ptr init config
  outputFrame_ptr->numCols = NEW_COLS;               // Setup -> NEW_COLS
  outputFrame_ptr->numRows = NEW_ROWS;               // Setup -> NEW_ROWS
  outputFrame_ptr->pData = &outputArray_ptr[0];          // Setup -> uint8_t bilinInterpOutput[NEW_FRAME] (64x64)

  // interp_t* interp_ptr init config
  interp_ptr->scaleX = SCALE_X;
  interp_ptr->scaleY = SCALE_Y;
  interp_ptr->outputStrideY = SCALE_X * SCALE_Y * RAW_COLS;
  interp_ptr->pCoefA = (float*)coef_A;
  interp_ptr->pCoefB = (float*)coef_B;
  interp_ptr->pCoefC = (float*)coef_C;
  interp_ptr->pCoefD = (float*)coef_D;

  float sFactor = (interp_ptr->scaleX * interp_ptr->scaleY);

  for (uint8_t row = 0; row < interp_ptr->scaleY; row++) {
    for (uint8_t col = 0; col < interp_ptr->scaleX; col++) {
      int index = row * interp_ptr->scaleX + col;
      interp_ptr->pCoefA[index] = (interp_ptr->scaleX - col) * (interp_ptr->scaleY - row) / sFactor;
      interp_ptr->pCoefB[index] = col * (interp_ptr->scaleY - row) / sFactor;
      interp_ptr->pCoefC[index] = (interp_ptr->scaleX - col) * row / sFactor;
      interp_ptr->pCoefD[index] = row * col / sFactor;
    }
  }
}

// Bilinear interpolation
void interp_matrix(
  uint8_t threshold,
  image_t* outputFrame_ptr,
  image_t* inputFrame_ptr,
  interp_t* interp_ptr
) {

  uint8_t inIndexA = 0;
  uint8_t inIndexB = 0;
  uint8_t inIndexC = 0;
  uint8_t inIndexD = 0;

  for (uint8_t rowPos = 0; rowPos < inputFrame_ptr->numRows - 1; rowPos++) {
    for (uint8_t colPos = 0; colPos < inputFrame_ptr->numCols - 1; colPos++) {

      inIndexA = rowPos * inputFrame_ptr->numCols + colPos;

      if (inputFrame_ptr->pData[inIndexA] > threshold) { // Windowing implementation

        inIndexB = inIndexA + 1;
        inIndexC = inIndexA + inputFrame_ptr->numCols;
        inIndexD = inIndexC + 1;

        for (uint8_t row = 0; row < interp_ptr->scaleY; row++) {
          for (uint8_t col = 0; col < interp_ptr->scaleX; col++) {

            uint8_t coefIndex = row * interp_ptr->scaleX + col;
            uint16_t outIndex = rowPos * interp_ptr->outputStrideY + colPos * interp_ptr->scaleX + row * outputFrame_ptr->numCols + col;

            outputFrame_ptr->pData[outIndex] =
              (uint8_t)round(
                inputFrame_ptr->pData[inIndexA] * interp_ptr->pCoefA[coefIndex] +
                inputFrame_ptr->pData[inIndexB] * interp_ptr->pCoefB[coefIndex] +
                inputFrame_ptr->pData[inIndexC] * interp_ptr->pCoefC[coefIndex] +
                inputFrame_ptr->pData[inIndexD] * interp_ptr->pCoefD[coefIndex]
              );
          }
        }
      }
      else {
        for (uint8_t row = 0; row < interp_ptr->scaleY; row++) {
          for (uint8_t col = 0; col < interp_ptr->scaleX; col++) {
            uint16_t outIndex = rowPos * interp_ptr->outputStrideY + colPos * interp_ptr->scaleX + row * outputFrame_ptr->numCols + col;
            outputFrame_ptr->pData[outIndex] = 0;
          }
        }
      }
    }
  }
}

void print_interp(image_t* image_ptr) {
  for (uint8_t posY = 0; posY < image_ptr->numRows; posY++) {
    uint8_t* row_ptr = COMPUTE_IMAGE_ROW_PTR(image_ptr, posY);
    for (int posX = 0; posX < image_ptr->numCols; posX++) {
      Serial.printf("%d-", IMAGE_GET_PIXEL_FAST(row_ptr, posX));
    }
    Serial.printf("\n");
  }
  Serial.printf("\n\n");
}
