/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

#if MIDI_HARDWARE
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);

void MIDI_SETUP() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
}
#endif

void getVelocity(llist_t* blobs_ptr, velocity_t* velocity_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    // Filtering
    uint8_t index = velocity_ptr[blob_ptr->UID].index++;
    velocity_ptr[blob_ptr->UID].windowX[index] = blob_ptr->centroid.X;
    //blob_ptr->centroid.Y;
    //blob_ptr->box.D[i];
  }
}

void gridLayoutSet() {
  //TODO
}

// Compute the grid Index location acording to blob X-Y positions
// Return curent & last keys presed
void gridLayoutPlay(
  llist_t* blobs_ptr,
  keyPos_t* key_ptr,
  uint8_t posX,
  uint8_t posY,
  uint8_t gridW,
  uint8_t gridH
) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    // Test if the blob is alive
    if (blob_ptr->alive) {
      // Test if the blob in the grid limits
      if (blob_ptr->centroid.X > posX && blob_ptr->centroid.X < posX + gridW &&
          blob_ptr->centroid.Y > posY && blob_ptr->centroid.Y < posY + gridH) {
        // Compute the X and Y key position
        uint8_t keyPosX = (uint8_t)round(((blob_ptr->centroid.X - posX) / gridW) * GRID_STEPS_X);
        uint8_t keyPosY = (uint8_t)round(((blob_ptr->centroid.Y - posY) / gridH) * GRID_STEPS_Y);
        key_ptr[blob_ptr->UID].keyPos = keyPosY * GRID_STEPS_Y + keyPosX;
        // TODO ADD DEBOUNCE / key_ptr[blob_ptr->UID].debounce & millis();
        if (key_ptr[blob_ptr->UID].keyPos != key_ptr[blob_ptr->UID].lastKeyPos &&
            millis() - key_ptr[blob_ptr->UID].debounce > DEBOUNCE_SWITCH_TIME) {
          key_ptr[blob_ptr->UID].debounce = millis();
          MIDI.sendNoteOff(key_ptr[blob_ptr->UID].lastKeyPos, 0, 1);  // Send a NoteOff
          MIDI.sendNoteOn(key_ptr[blob_ptr->UID].keyPos, 127, 1);     // Send a NoteOn
#if DEBUG_MAPPING
          Serial.printf("\nKEY_OFF : %d \t KEY_ON : %d", key_ptr[blob_ptr->UID].lastKeyPos, key_ptr[blob_ptr->UID].keyPos);
#endif
          key_ptr[blob_ptr->UID].lastKeyPos = key_ptr[blob_ptr->UID].keyPos;
        }
      }
    }
    else {
      key_ptr[blob_ptr->UID].lastKeyPos = key_ptr[blob_ptr->UID].keyPos;
      MIDI.sendNoteOff(key_ptr[blob_ptr->UID].keyPos, 0, 1);  // Send a Note (pitch 42, velo 127 on channel 1)
#if DEBUG_MAPPING
      Serial.printf("\nKEY_OFF : %d", key_ptr[blob_ptr->UID].keyPos);
#endif
      key_ptr[blob_ptr->UID].keyPos = NULL;
      key_ptr[blob_ptr->UID].lastKeyPos = NULL;
    }
  }
}

uint16_t harmonicKeyboardLayout(
  blob_t* blob_ptr,
  keyPos_t* key_ptr,
  uint8_t posX,
  uint8_t posY,
  uint8_t gridW,
  uint8_t gridH
) {
  // TODO
}

// FIXME
void vSlider(llist_t* blobs_ptr, sliderV_t* slider_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > slider_ptr->posX - slider_ptr->width &&
        blob_ptr->centroid.X < slider_ptr->posX + slider_ptr->width) {
      if (blob_ptr->centroid.Y > slider_ptr->Ymin &&
          blob_ptr->centroid.Y < slider_ptr->Ymax) {
        slider_ptr->val = (uint8_t)round((blob_ptr->centroid.Y - (float)slider_ptr->Ymin) * ((slider_ptr->Ymax - slider_ptr->Ymin) / 127)); // [0:127]
#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_V_SLIDER : %d", slider_ptr->val);
#endif
      }
    }
  }
}

// FIXME
void hSlider(llist_t* blobs_ptr, sliderH_t* slider_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.Y > slider_ptr->posY - slider_ptr->height &&
        blob_ptr->centroid.Y < slider_ptr->posY + slider_ptr->height) {
      if (blob_ptr->centroid.X > slider_ptr->Xmin &&
          blob_ptr->centroid.X < slider_ptr->Xmax) {
        slider_ptr->val = (uint8_t)round((blob_ptr->centroid.Y - (float)slider_ptr->Xmin) * ((slider_ptr->Xmax - slider_ptr->Xmin) / 127)); // [0:127]
#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_V_SLIDER : %d", slider_ptr->val);
#endif
      }
    }
  }
}

void cSlider(llist_t* blobs_ptr, polar_t* polarPos_ptr, float radius, float phiMin, float phiMax, uint8_t wSize) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (polarPos_ptr[blob_ptr->UID].r > radius - wSize && polarPos_ptr[blob_ptr->UID].r < radius + wSize) {
      if ( polarPos_ptr[blob_ptr->UID].phi > phiMin && polarPos_ptr[blob_ptr->UID].phi < phiMin) {
        //uint8_t sliderVal = (blob.phi - phiMin) * ((phiMax - phiMin) / 127); // [0:127] FIXME
#if DEBUG_MAPPING
        //Serial.printf("\nDEBUG_C_SLIDER : %f", sliderVal);
#endif
      }
    }
  }
}

void getPolarCoordinates(llist_t* blobs_ptr, polar_t* polarPos_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    float posX = blob_ptr->centroid.X - POLAR_X;
    float posY = blob_ptr->centroid.Y - POLAR_Y;
    if (posX == 0 && posY == 0 ) {
      polarPos_ptr[blob_ptr->UID].r = 0;
      polarPos_ptr[blob_ptr->UID].phi = 0;
    }
    else {
      polarPos_ptr[blob_ptr->UID].r = sqrt(posX * posX + posY * posY);
      if (posX == 0 && 0 < posY) {
        polarPos_ptr[blob_ptr->UID].phi = PI / 2;
      } else if (posX == 0 && posY < 0) {
        polarPos_ptr[blob_ptr->UID].phi = PI * 3 / 2;
      } else if (posX < 0) {
        polarPos_ptr[blob_ptr->UID].phi = atan(posY / posX) + PI;
      } else if (posY < 0) {
        polarPos_ptr[blob_ptr->UID].phi = atan(posY / posX) + 2 * PI;
      } else {
        polarPos_ptr[blob_ptr->UID].phi = atan(posY / posX);
      }
#if DEBUG_MAPPING
      Serial.printf("\nDEBUG_POLAR : R:%f\tPHY:%f", polarPos_ptr[blob_ptr->UID].r, polarPos_ptr[blob_ptr->UID].phi);
#endif
    }
  }
}

boolean toggle(llist_t* blobs_ptr, switch_t* tSwitch) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > tSwitch->posX - tSwitch->rSize && blob_ptr->centroid.X < tSwitch->posX + tSwitch->rSize) {
      if (blob_ptr->centroid.Y > tSwitch->posY - tSwitch->rSize && blob_ptr->centroid.Y < tSwitch->posY + tSwitch->rSize) {
        if (millis() - tSwitch->debounce > DEBOUNCE_SWITCH_TIME) {
          tSwitch->debounce = millis();
          tSwitch->state = !tSwitch->state;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_TOGGLE : STATE:%d", tSwitch->state);
#endif
          return tSwitch->state;
        }
        tSwitch->debounce = millis();
      }
    }
  }
}

boolean trigger(blob_t* blob_ptr, switch_t* tSwitch) {

  if (blob_ptr->centroid.X > tSwitch->posX - tSwitch->rSize && blob_ptr->centroid.X < tSwitch->posX + tSwitch->rSize) {
    if (blob_ptr->centroid.Y > tSwitch->posY - tSwitch->rSize && blob_ptr->centroid.Y < tSwitch->posY + tSwitch->rSize) {
      if (millis() - tSwitch->debounce > DEBOUNCE_SWITCH_TIME) {
        tSwitch->debounce = millis();
#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_TRIGGER : POSX:%f\tPOSY:%f", blob_ptr->centroid.X, blob_ptr->centroid.Y);
#endif
        return true;
      }
      tSwitch->debounce = millis();
    }
  }
  else {
    return false;
  }
}
