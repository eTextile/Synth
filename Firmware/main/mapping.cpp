/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

#if MAPPING_LAYAOUT

#define GRID_X_SCAL_FACTOR    (float)(GRID_COLS * (1/X_MAX))
#define GRID_Y_SCAL_FACTOR    (float)(GRID_ROWS * (1/Y_MAX))
#define DEBOUNCE_TIME_SWITCH  15            // (TO_REMOVE) this is done in find_blobs();

squareKey_t keys[GRID_KEYS];                // 1D array to store keys limits
squareKey_t* keyPress_ptr[MAX_SYNTH];       // 1D pointer array to store last keysPressed pointer
squareKey_t* lastKeyPress_ptr[MAX_SYNTH];   // 1D pointer array to store last keysPressed pointer

uint8_t freqKeyLayout[GRID_KEYS] = {0};     // 1D array to mapp freq
midiNode_t midiKeyLayout[GRID_KEYS] = {0};  // 1D array to mapp incoming midi notes in the grid layout

void mapping_trigger(tSwitch_t* switch_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > switch_ptr->posX - switch_ptr->rSize &&
        blob_ptr->centroid.X < switch_ptr->posX + switch_ptr->rSize) {
      if (blob_ptr->centroid.Y > switch_ptr->posY - switch_ptr->rSize &&
          blob_ptr->centroid.Y < switch_ptr->posY + switch_ptr->rSize) {
        if (millis() - switch_ptr->timeStamp > DEBOUNCE_TIME_SWITCH) {
          switch_ptr->timeStamp = millis();
          switch_ptr->state = true;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_TRIGGER : POSX: % f\tPOSY: % f", blob_ptr->centroid.X, blob_ptr->centroid.Y);
#endif
          //return true;
        };
        switch_ptr->timeStamp = millis();
      };
    }
    else {
      //return false;
    };
  };
};

void mapping_toggle(tSwitch_t* switch_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > switch_ptr->posX - switch_ptr->rSize &&
        blob_ptr->centroid.X < switch_ptr->posX + switch_ptr->rSize) {
      if (blob_ptr->centroid.Y > switch_ptr->posY - switch_ptr->rSize &&
          blob_ptr->centroid.Y < switch_ptr->posY + switch_ptr->rSize) {
        if (millis() - switch_ptr->timeStamp > DEBOUNCE_TIME_SWITCH) {
          switch_ptr->timeStamp = millis();
          switch_ptr->state = !switch_ptr->state;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_TOGGLE : STATE: % d", switch_ptr->state);
#endif
          //return switch_ptr->state;
        };
        switch_ptr->timeStamp = millis();
      };
    };
  };
};

// Pre-compute key min & max coordinates
void GRID_LAYOUT_SETUP(void) {

  //Serial.printf("\nKEY_SIZE_X:%d \tKEY_SIZE_Y:%d", KEY_SIZE_X, KEY_SIZE_Y);
  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      uint8_t index = row * GRID_COLS + col;

      if (index < 127) keys[index].val =  index; // FIXME!
      //keys[index].val = (int8_t)harmonicKeyboard[index];

      keys[index].Xmin = col * KEY_SIZE_X + ((col + 1) * GRID_GAP);
      keys[index].Xmax = keys[index].Xmin + KEY_SIZE_X;
      keys[index].Ymin = row * KEY_SIZE_Y + ((row + 1) * GRID_GAP);
      keys[index].Ymax = keys[index].Ymin + KEY_SIZE_Y;
#if DEBUG_MAPPING
      Serial.printf("\nGRID\tXmin:%d\tXmax:%d\tYmin:%d\tYmax:%d",
                    keys[index].Xmin, keys[index].Xmax,
                    keys[index].Ymin, keys[index].Ymax
                   );
#endif
    };
  };
};

// Compute the grid index location acording to the blobs XY (centroid) coordinates
// Play corresponding midi **note** or **freq**
void mapping_gridPlay(void) {

  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {                                          // Test if the blob UID is less than MAX_SYNTH

      int keyPosX = round(blob_ptr->centroid.X * GRID_X_SCAL_FACTOR); // Compute X grid position
      int keyPosY = round(blob_ptr->centroid.Y * GRID_Y_SCAL_FACTOR); // Compute Y grid position
      int index = keyPosY * GRID_COLS + keyPosX;                      // Compute 1D key index position

      if (!blob_ptr->lastState) lastKeyPress_ptr[blob_ptr->UID] = NULL;
      if (blob_ptr->state) {
        if (&keys[index] != lastKeyPress_ptr[blob_ptr->UID]) {
          // Test if the blob is within the key limits
          if (blob_ptr->centroid.X > keys[index].Xmin && blob_ptr->centroid.X < keys[index].Xmax &&
              blob_ptr->centroid.Y > keys[index].Ymin && blob_ptr->centroid.Y < keys[index].Ymax) {
            if (lastKeyPress_ptr[blob_ptr->UID] != NULL) {
#if DEBUG_MAPPING
              Serial.printf("\nGRID\tBLOB:%d\t\tKEY_OFF:%d", blob_ptr->UID, lastKeyPress_ptr[blob_ptr->UID]->val);
#else
              /*
                //MIDI.sendNoteOff(lastKeyPress_ptr[blob_ptr->UID]->val, 0, 1);  // Send NoteOFF (CHANNEL_1)
                midiNode_t* node = (midiNode_t*)llist_pop_front(&llist_midiNodes_stack);
                node->pithch = 0;
                node->velocity = 0;
                //node->channel = blob_ptr->UID;
                node->channel = 1;
                llist_push_front(&midiOut, node);
              */
#endif
            };
#if DEBUG_MAPPING
            Serial.printf("\nGRID\tBLOB:%d\t\tKEY_DOWN:%d", blob_ptr->UID, keys[index].val);
#else
            //MIDI.sendNoteOn(keyPress_ptr->val, 127, 1); // Send NoteON (CHANNEL_1)
#endif
            lastKeyPress_ptr[blob_ptr->UID] = &keys[index];
          };
        };
      }
      else {
#if DEBUG_MAPPING
        Serial.printf("\nGRID\tBLOB:%d\t\tKEY_UP:%d", blob_ptr->UID, keys[index].val);
#else
        //MIDI.sendNoteOff(lastKeyPress_ptr[blob_ptr->UID]->val, 0, 1);  // Send NoteOFF (CHANNEL_1)
#endif
      };
    };
  };
};

void mapping_gridPopulate(void) {
  uint8_t keyIndex = 0;
  while (keyIndex < GRID_KEYS) {
    for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&blobs); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
      midiKeyLayout[keyIndex].pithch = node_ptr->pithch;
      midiKeyLayout[keyIndex].velocity = node_ptr->velocity;
      midiKeyLayout[keyIndex].channel = node_ptr->channel;
      keyIndex++;
    };
  };
};

void mapping_vSlider(vSlider_t* slider_ptr) {
  uint8_t val = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > slider_ptr->posX - slider_ptr->width &&
        blob_ptr->centroid.X < slider_ptr->posX + slider_ptr->width) {
      if (blob_ptr->centroid.Y > slider_ptr->Ymin &&
          blob_ptr->centroid.Y < slider_ptr->Ymax) {
        val = round(map(blob_ptr->centroid.Y, slider_ptr->Ymin, slider_ptr->Ymax, 0, 127)); // [0:127]
        if (val != slider_ptr->val) {
          slider_ptr->val = val;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_V_SLIDER : % d", val);
#endif
        };
      };
    };
  };
};

void mapping_hSlider(hSlider_t* slider_ptr) {
  uint8_t val = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.Y > slider_ptr->posY - slider_ptr->height &&
        blob_ptr->centroid.Y < slider_ptr->posY + slider_ptr->height) {
      if (blob_ptr->centroid.X > slider_ptr->Xmin &&
          blob_ptr->centroid.X < slider_ptr->Xmax) {
        val = round(map(blob_ptr->centroid.X, slider_ptr->Xmin, slider_ptr->Xmax, 0, 127)); // [0:127]
        if (val != slider_ptr->val) {
          slider_ptr->val = val;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_H_SLIDER : % d", val);
#endif
        };
      };
    };
  };
};

/*
  float r;
  uint8_t width;
  float phiOffset;
  float phiMin;
  float phiMax;
  uint8_t val;
*/

void mapping_cSliders(cSlider_t* cSliders_ptr) {
  float phi = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (int i = 0; i < C_SLIDERS; i++) {

      if (blob_ptr->polar.r > cSliders_ptr[i].r - cSliders_ptr[i].width &&
          blob_ptr->polar.r < cSliders_ptr[i].r + cSliders_ptr[i].width) {

        if (blob_ptr->polar.phi > cSliders_ptr[i].phiMin &&
            blob_ptr->polar.phi < cSliders_ptr[i].phiMax) {
          cSliders_ptr[i].val = blob_ptr->polar.phi - cSliders_ptr[i].phiOffset;
        }
        else {
          phi = blob_ptr->polar.phi + (PI2 - cSliders_ptr[i].phiOffset);
        };
#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_C_SLIDER_%d - phi:%f", i, cSliders_ptr[i].val );
#endif
      };
    };
  };
};


// ccPesets_ptr -> ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]
void mapping_cChange(cChange_t* cChange_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    // Test if we are within the blob limit
    if (blob_ptr->UID == cChange_ptr->blobID) {
      // Test if the blob is alive
      if (blob_ptr->state) {
        switch (cChange_ptr->mappVal) {
          case BX:
            if (blob_ptr->centroid.X != cChange_ptr->val) {
              cChange_ptr->val = blob_ptr->centroid.X;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.X, 0, 127), cChange_ptr->midiChannel);
#else
              //MIDI.sendControlChange(cChange_ptr->cChange, constrain(blob_ptr->centroid.X, 0, 127), cChange_ptr->midiChannel);
#endif
            };
            break;
          case BY:
            if (blob_ptr->centroid.Y != cChange_ptr->val) {
              cChange_ptr->val = blob_ptr->centroid.Y;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.Y, 0, 127), ccParams_ptr->midiChannel);
#else
              //MIDI.sendControlChange(cChange_ptr->cChange, constrain(blob_ptr->centroid.Y, 0, 127), cChange_ptr->midiChannel);
#endif
            };
            break;
          case BW:
            if (blob_ptr->box.W != cChange_ptr->val) {
              cChange_ptr->val = blob_ptr->box.W;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.W, 0, 127), ccParams_ptr->midiChannel);
#else
              //MIDI.sendControlChange(cChange_ptr->cChange, constrain(blob_ptr->box.W, 0, 127), cChange_ptr->midiChannel);
#endif
            };
            break;
          case BH:
            if (blob_ptr->box.H != cChange_ptr->val) {
              cChange_ptr->val = blob_ptr->box.H;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.H, 0, 127), ccPesets_ptr->midiChannel);
#else
              //MIDI.sendControlChange(cChange_ptr->cChange, constrain(blob_ptr->box.H, 0, 127), cChange_ptr->midiChannel);
#endif
            };
            break;
          case BD:
            if (blob_ptr->box.D != cChange_ptr->val) {
              cChange_ptr->val = blob_ptr->box.D;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nBLOB:%d\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, cChange_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), cChange_ptr->midiChannel);
#else
              //MIDI.sendControlChange(cChange_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), cChange_ptr->midiChannel);
#endif
            };
            break;
        };
      };
    };
  };
};
#endif
