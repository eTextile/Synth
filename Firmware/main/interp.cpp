/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
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

void SETUP_INTERP(
  image_t* inputFrame_ptr,
  uint8_t* inputArray_ptr,
  image_t* outputFrame,
  uint8_t* outputArray,
  interp_t* interp
) {

  // image_t* inputFrame init config
  inputFrame_ptr->numCols = RAW_COLS;            // Setup -> RAW_COLS
  inputFrame_ptr->numRows = RAW_ROWS;            // Setup -> RAW_ROWS
  inputFrame_ptr->pData = &inputArray_ptr[0];    // Setup -> uint8_t frameArray[RAW_FRAME] (16x16)

  // image_t* outputFrame init config
  outputFrame->numCols = NEW_COLS;               // Setup -> NEW_COLS
  outputFrame->numRows = NEW_ROWS;               // Setup -> NEW_ROWS
  outputFrame->pData = &outputArray[0];          // Setup -> uint8_t bilinInterpOutput[NEW_FRAME] (64x64)

  // interp_t* interp init config
  interp->scale_X = SCALE_X;
  interp->scale_Y = SCALE_Y;
  interp->outputStride_Y = SCALE_X * SCALE_Y * RAW_COLS;
  interp->pCoefA = (float*)coef_A;
  interp->pCoefB = (float*)coef_B;
  interp->pCoefC = (float*)coef_C;
  interp->pCoefD = (float*)coef_D;

  float sFactor = (interp->scale_X * interp->scale_Y);

  for (uint8_t row = 0; row < interp->scale_Y; row++) {
    for (uint8_t col = 0; col < interp->scale_X; col++) {
      int index = row * interp->scale_X + col;
      interp->pCoefA[index] = (interp->scale_X - col) * (interp->scale_Y - row) / sFactor;
      interp->pCoefB[index] = col * (interp->scale_Y - row) / sFactor;
      interp->pCoefC[index] = (interp->scale_X - col) * row / sFactor;
      interp->pCoefD[index] = row * col / sFactor;
    }
  }
}

// Bilinear interpolation
void interp_matrix(
  uint8_t interpThreshold,
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

      if (inputFrame_ptr->pData[inIndexA] > interpThreshold) { // Windowing implementation

        inIndexB = inIndexA + 1;
        inIndexC = inIndexA + inputFrame_ptr->numCols;
        inIndexD = inIndexC + 1;

        for (uint8_t row = 0; row < interp_ptr->scale_Y; row++) {
          for (uint8_t col = 0; col < interp_ptr->scale_X; col++) {

            uint8_t coefIndex = row * interp_ptr->scale_X + col;
            uint16_t outIndex = rowPos * interp_ptr->outputStride_Y + colPos * interp_ptr->scale_X + row * outputFrame_ptr->numCols + col;

            outputFrame_ptr->pData[outIndex] =
              (uint8_t) round(
                inputFrame_ptr->pData[inIndexA] * interp_ptr->pCoefA[coefIndex] +
                inputFrame_ptr->pData[inIndexB] * interp_ptr->pCoefB[coefIndex] +
                inputFrame_ptr->pData[inIndexC] * interp_ptr->pCoefC[coefIndex] +
                inputFrame_ptr->pData[inIndexD] * interp_ptr->pCoefD[coefIndex]
              );
          }
        }
      }
      else {
        for (uint8_t row = 0; row < interp_ptr->scale_Y; row++) {
          for (uint8_t col = 0; col < interp_ptr->scale_X; col++) {
            uint16_t outIndex = rowPos * interp_ptr->outputStride_Y + colPos * interp_ptr->scale_X + row * outputFrame_ptr->numCols + col;
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
