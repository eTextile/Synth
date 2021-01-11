/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "mapping.h"

void velocity(blob_t* blob_ptr) {
  // TODO : xyz velocity
}

// Return the grid X-Y location acording to blob X-Y position
keyCode_t gridLayout(blob_t* blob_ptr, uint8_t posX, uint8_t posY, uint8_t gridW, uint8_t gridH, uint8_t stepX, uint8_t stepY) {

  keyCode_t key;

  // Test if we are in the grid limit
  if (blob_ptr->centroid.X > posX && blob_ptr->centroid.X < posX + gridW &&
      blob_ptr->centroid.Y > posY && blob_ptr->centroid.Y < posY + gridH) {
    uint8_t posX = 0;
    uint8_t posY = 0;

    // Look for the X and Y cell position
    key.posX = (uint8_t)round(((blob_ptr->centroid.X - posX) / gridW) * stepX);
    key.posY = (uint8_t)round(((blob_ptr->centroid.Y - posY) / gridH) * stepY);

    return key;
  }
}

void harmonicKeyboardLayout(blob_t* blob_ptr) {
  // TODO
}

void hSlider() {
  // TODO
}

void vSlider() {
  // TODO
}

void cSlider() {
  // TODO
}

void toggle() {
  // TODO
}

void trigger() {
  // TODO
}

polar_t polarCoordinates(blob_t* blob_ptr) {

  polar_t coord;

  if (blob_ptr->centroid.X == 0 && blob_ptr->centroid.Y == 0 ) {
    coord.r = 0;
    coord.phi = 0;
    return coord;
  }

  coord.r = sqrt(blob_ptr->centroid.X * blob_ptr->centroid.X + blob_ptr->centroid.Y * blob_ptr->centroid.Y);

  if (blob_ptr->centroid.X == 0 && 0 < blob_ptr->centroid.Y) {
    coord.phi = PI / 2;
  } else if (blob_ptr->centroid.X == 0 && blob_ptr->centroid.Y < 0) {
    coord.phi = PI * 3 / 2;
  } else if (blob_ptr->centroid.X < 0) { //x != 0
    coord.phi = atan(blob_ptr->centroid.Y / blob_ptr->centroid.X) + PI;
  } else if (blob_ptr->centroid.Y < 0) {
    coord.phi = atan(blob_ptr->centroid.Y / blob_ptr->centroid.X) + 2 * PI;
  } else {
    coord.phi = atan(blob_ptr->centroid.Y / blob_ptr->centroid.X);
  }
  return coord;
}
