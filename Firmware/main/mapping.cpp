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

void getVelocity(llist_t* blobs_ptr, velocity_t* storage_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {

    storage_ptr[blob_ptr->UID].velocityX = blob_ptr->centroid.X - storage_ptr[blob_ptr->UID].lastValX;
    storage_ptr[blob_ptr->UID].velocityY = blob_ptr->centroid.Y - storage_ptr[blob_ptr->UID].lastValY;
    storage_ptr[blob_ptr->UID].velocityD = blob_ptr->box.D - storage_ptr[blob_ptr->UID].lastValD;

    storage_ptr[blob_ptr->UID].lastValX = blob_ptr->centroid.X;
    storage_ptr[blob_ptr->UID].lastValY = blob_ptr->centroid.Y;
    storage_ptr[blob_ptr->UID].lastValD = blob_ptr->box.D;

#if DEBUG_MAPPING
    Serial.printf("\nDEBUG_VELOCITY : X:%f\tY:%f\tZ:%f", storage_ptr[blob_ptr->UID].velocityX, storage_ptr[blob_ptr->UID].velocityY, storage_ptr[blob_ptr->UID].velocityD);
#endif
  }
}

void gridLayoutSet() {
  //TODO
}

// Compute the grid Index location acording to blob X-Y positions
// Return curent & last keys presed
void gridLayoutPlay( llist_t* blobs_ptr, keyPos_t* key_ptr, grid_t* grid_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_BLOBS) {
      // Test if the blob is alive
      if (blob_ptr->alive) {
        // Test if the blob is in the grid limits
        if (blob_ptr->centroid.X > grid_ptr->posX && blob_ptr->centroid.X < grid_ptr->posX + grid_ptr->W &&
            blob_ptr->centroid.Y > grid_ptr->posY && blob_ptr->centroid.Y < grid_ptr->posY + grid_ptr->H) {
          // Compute X and Y key position
          uint8_t keyPosX = (uint8_t)round(((blob_ptr->centroid.X - grid_ptr->posX) / grid_ptr->W) * GRID_STEPS_X);
          uint8_t keyPosY = (uint8_t)round(((blob_ptr->centroid.Y - grid_ptr->posY) / grid_ptr->H) * GRID_STEPS_Y);
          // Compute 1D index key position
          key_ptr[blob_ptr->UID].keyPos = keyPosY * GRID_STEPS_Y + keyPosX;
          if (key_ptr[blob_ptr->UID].keyPos != key_ptr[blob_ptr->UID].lastKeyPos &&
              millis() - key_ptr[blob_ptr->UID].debounceTime > DEBOUNCE_TIME_SWITCH) {
            key_ptr[blob_ptr->UID].debounceTime = millis();
#if MIDI_HARDWARE
            MIDI.sendNoteOff(key_ptr[blob_ptr->UID].lastKeyPos, 0, blob_ptr->UID + 1);  // Send a NoteOff
            MIDI.sendNoteOn(key_ptr[blob_ptr->UID].keyPos, 127, blob_ptr->UID + 1);     // Send a NoteOn
#endif
#if MIDI_USB
            usbMIDI.sendNoteOff(key_ptr[blob_ptr->UID].lastKeyPos, 0, blob_ptr->UID + 1);  // Send a NoteOff
            usbMIDI.sendNoteOn(key_ptr[blob_ptr->UID].keyPos, 127, blob_ptr->UID + 1);     // Send a NoteOn
#endif
#if DEBUG_MAPPING
            Serial.printf("\nKEY_OFF : %d\tKEY_ON : %d", key_ptr[blob_ptr->UID].lastKeyPos, key_ptr[blob_ptr->UID].keyPos);
#endif
            key_ptr[blob_ptr->UID].lastKeyPos = key_ptr[blob_ptr->UID].keyPos;
          }
        }
      }
      else {
        key_ptr[blob_ptr->UID].lastKeyPos = key_ptr[blob_ptr->UID].keyPos;
#if MIDI_HARDWARE
        MIDI.sendNoteOff(key_ptr[blob_ptr->UID].keyPos, 0, blob_ptr->UID + 1);  // Send a Note (pitch 42, velo 127 on channel 1)
#endif
#if MIDI_USB
        usbMIDI.sendNoteOff(key_ptr[blob_ptr->UID].keyPos, 0, blob_ptr->UID + 1);  // Send a Note (pitch 42, velo 127 on channel 1)
#endif
#if DEBUG_MAPPING
        Serial.printf("\nKEY_OFF : %d", key_ptr[blob_ptr->UID].keyPos);
#endif
        key_ptr[blob_ptr->UID].keyPos = NULL;
        key_ptr[blob_ptr->UID].lastKeyPos = NULL;
      }
    }
  }
}

// TODO
void harmonicKeyboardLayout( blob_t* blob_ptr, keyPos_t* key_ptr, grid_t* grid_ptr) {

}

void vSlider(llist_t* blobs_ptr, vSlider_t* slider_ptr) {

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
void hSlider(llist_t* blobs_ptr, hSlider_t* slider_ptr) {

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

void cSlider(llist_t* blobs_ptr, polar_t* polar_ptr, cSlider_t* slider_ptr) {
  float phi = 0;

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    for (int i = 0; i < C_SLIDERS; i++) {
      if (polar_ptr[blob_ptr->UID].r > slider_ptr[i].r - slider_ptr[i].width &&
          polar_ptr[blob_ptr->UID].r < slider_ptr[i].r + slider_ptr[i].width) {
        if (polar_ptr[blob_ptr->UID].phi > slider_ptr[i].phiOffset) {
          phi = polar_ptr[blob_ptr->UID].phi - slider_ptr[i].phiOffset;
        }
        else {
          phi = polar_ptr[blob_ptr->UID].phi + (6.2839 - slider_ptr[i].phiOffset);
        }
#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_C_SLIDER_%d phi : %f", i, map(constrain(phi, 0.2, 5.9), 0.2, 5.9, 0, 127));
#endif
      }
    }
  }
}

void getPolarCoordinates(llist_t* blobs_ptr, polar_t* polar_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    float posX = blob_ptr->centroid.X - POLAR_CX;
    float posY = blob_ptr->centroid.Y - POLAR_CY;
    if (posX == 0 && posY == 0 ) {
      polar_ptr[blob_ptr->UID].r = 0;
      polar_ptr[blob_ptr->UID].phi = 0;
    }
    else {
      polar_ptr[blob_ptr->UID].r = sqrt(posX * posX + posY * posY);
      if (posX == 0 && 0 < posY) {
        polar_ptr[blob_ptr->UID].phi = PI / 2;
      } else if (posX == 0 && posY < 0) {
        polar_ptr[blob_ptr->UID].phi = PI * 3 / 2;
      } else if (posX < 0) {
        polar_ptr[blob_ptr->UID].phi = atan(posY / posX) + PI;
      } else if (posY < 0) {
        polar_ptr[blob_ptr->UID].phi = atan(posY / posX) + 2 * PI;
      } else {
        polar_ptr[blob_ptr->UID].phi = atan(posY / posX);
      }
    }
#if DEBUG_MAPPING
    Serial.printf("\nDEBUG_POLAR : R:%f\tPHY:%f", polar_ptr[blob_ptr->UID].r, polar_ptr[blob_ptr->UID].phi);
#endif
  }
}

boolean toggle(llist_t* blobs_ptr, tSwitch_t* tSwitch) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > tSwitch->posX - tSwitch->rSize && blob_ptr->centroid.X < tSwitch->posX + tSwitch->rSize) {
      if (blob_ptr->centroid.Y > tSwitch->posY - tSwitch->rSize && blob_ptr->centroid.Y < tSwitch->posY + tSwitch->rSize) {
        if (millis() - tSwitch->debounceTime > DEBOUNCE_TIME_SWITCH) {
          tSwitch->debounceTime = millis();
          tSwitch->state = !tSwitch->state;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_TOGGLE : STATE:%d", tSwitch->state);
#endif
          return tSwitch->state;
        }
        tSwitch->debounceTime = millis();
      }
    }
  }
}

boolean trigger(blob_t* blob_ptr, tSwitch_t* tSwitch) {

  if (blob_ptr->centroid.X > tSwitch->posX - tSwitch->rSize && blob_ptr->centroid.X < tSwitch->posX + tSwitch->rSize) {
    if (blob_ptr->centroid.Y > tSwitch->posY - tSwitch->rSize && blob_ptr->centroid.Y < tSwitch->posY + tSwitch->rSize) {
      if (millis() - tSwitch->debounceTime > DEBOUNCE_TIME_SWITCH) {
        tSwitch->debounceTime = millis();
        tSwitch->state = true;
#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_TRIGGER : POSX:%f\tPOSY:%f", blob_ptr->centroid.X, blob_ptr->centroid.Y);
#endif
        return true;
      }
      tSwitch->debounceTime = millis();
    }
  }
  else {
    return false;
  }
}
