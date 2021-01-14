/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

boolean toggle(blob_t* blob_ptr, switch_t* tSwitch) {

  if (blob_ptr->centroid.X > tSwitch->posX - tSwitch->rSize && blob_ptr->centroid.X < tSwitch->posX + tSwitch->rSize) {
    if (blob_ptr->centroid.Y > tSwitch->posY - tSwitch->rSize && blob_ptr->centroid.Y < tSwitch->posY + tSwitch->rSize) {
      if (tSwitch->debounceTimer > MAPP_DEBOUNCE_TIME) {
        tSwitch->debounceTimer = 0;
        tSwitch->state = !tSwitch->state;
        return tSwitch->state;
      }
    }
  }
}

boolean trigger(blob_t* blob_ptr, switch_t* tSwitch) {

  if (blob_ptr->centroid.X > tSwitch->posX - tSwitch->rSize && blob_ptr->centroid.X < tSwitch->posX + tSwitch->rSize) {
    if (blob_ptr->centroid.Y > tSwitch->posY - tSwitch->rSize && blob_ptr->centroid.Y < tSwitch->posY + tSwitch->rSize) {
      if (tSwitch->debounceTimer > MAPP_DEBOUNCE_TIME) {
        tSwitch->debounceTimer = 0;
        return true;
      }
    }
  }
  else {
    return false;
  }
}

void velocity(blob_t* blob_ptr) {
  // TODO : xyz velocity
}

// Return the grid X-Y location acording to blob X-Y position
keyCode_t gridLayout(blob_t* blob_ptr, uint8_t gridW, uint8_t gridH, uint8_t stepX, uint8_t stepY, uint8_t posX, uint8_t posY) {

  keyCode_t key;

  // Test if we are in the grid limit
  if (blob_ptr->centroid.X > posX && blob_ptr->centroid.X < posX + gridW &&
      blob_ptr->centroid.Y > posY && blob_ptr->centroid.Y < posY + gridH) {
    // Look for the X and Y cell position
    key.posX = (uint8_t)round(((blob_ptr->centroid.X - posX) / gridW) * stepX);
    key.posY = (uint8_t)round(((blob_ptr->centroid.Y - posY) / gridH) * stepY);
    Serial.printf("\n DEBUG_GRID : posX:%d \t posY:%d", key.posX, key.posY);
    return key;
  }
}

void harmonicKeyboardLayout(blob_t* blob_ptr) {
  // TODO
}

void vSlider(blob_t* blob_ptr, uint8_t posX, uint8_t Ymin, uint8_t Ymax, uint8_t width) {
  if (blob_ptr->centroid.X > posX - width / 2 && blob_ptr->centroid.X < posX + width / 2) {
    if (blob_ptr->centroid.Y > Ymin && blob_ptr->centroid.Y < Ymax) {
      //return val [0:127]
    }
  }
}

void hSlider(blob_t* blob_ptr, uint8_t posY, uint8_t Xmin, uint8_t Xmax, uint8_t height) {
  if (blob_ptr->centroid.Y > posY - height / 2 && blob_ptr->centroid.Y < posY + height / 2) {
    if (blob_ptr->centroid.X > Xmin && blob_ptr->centroid.X < Xmax) {
      //return val [0:127]
    }
  }
}

void cSlider(blob_t* blob_ptr, float radius, float tetaMin, float tetaMax, uint8_t width) {

  polar_t blob = polarCoordinates(blob_ptr, POLAR_X, POLAR_Y);

  if ( blob.r > radius - width / 2 && blob.r < radius + width / 2) {
    if ( blob.phi > tetaMin && blob.phi < tetaMin) {
      //return val [0:127]
    }
  }
}

polar_t polarCoordinates(blob_t* blob_ptr, uint8_t Xcenter, uint8_t Ycenter) {

  polar_t coord;

  float posX = blob_ptr->centroid.X - Xcenter;
  float posY = blob_ptr->centroid.Y - Ycenter;

  if (posX == 0 && posY == 0 ) {
    coord.r = 0;
    coord.phi = 0;
    return coord;
  }

  coord.r = sqrt(posX * posX + posY * posY);

  if (posX == 0 && 0 < posY) {
    coord.phi = PI / 2;
  } else if (posX == 0 && posY < 0) {
    coord.phi = PI * 3 / 2;
  } else if (posX < 0) {
    coord.phi = atan(posY / posX) + PI;
  } else if (posY < 0) {
    coord.phi = atan(posY / posX) + 2 * PI;
  } else {
    coord.phi = atan(posY / posX);
  }
  //Serial.printf("\nDEBUG_POLAR : R:%f\tPHY:%f", coord.r, coord.phi);
  return coord;
}
