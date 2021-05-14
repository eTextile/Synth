/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

#define GRID_COLS   8
#define GRID_ROWS   8
#define GRID_KEYS   (GRID_COLS * GRID_ROWS)
#define GRID_GAP    (float)0.5
#define KEY_SIZE    (float)(((X_MAX - ((GRID_COLS + 1) * GRID_GAP))) / GRID_COLS)

#define SWITCH_DEBOUNCE_TIMETCH    15

squareKey_t keyPos[GRID_KEYS] = {0};       // 1D array of struct squareKey_t to store pre-compute key positions ARGS[Xmin, Xmax, Ymin, Ymax]
uint8_t lastKeyPress[MAX_SYNTH] = {0};     // 1D array to store last keys pressed

uint8_t freqKeyLayout[GRID_KEYS] = {0};    // 1D array to mapp freq
midiNode_t midiKeyLayout[GRID_KEYS] = {0}; // 1D array to mapp incoming midi notes in the grid layout

boolean trigger(llist_t* llist_ptr, tSwitch_t* switch_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > switch_ptr->posX - switch_ptr->rSize &&
        blob_ptr->centroid.X < switch_ptr->posX + switch_ptr->rSize) {
      if (blob_ptr->centroid.Y > switch_ptr->posY - switch_ptr->rSize &&
          blob_ptr->centroid.Y < switch_ptr->posY + switch_ptr->rSize) {
        if (millis() - switch_ptr->timeStamp > SWITCH_DEBOUNCE_TIMETCH) {
          switch_ptr->timeStamp = millis();
          switch_ptr->state = true;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_TRIGGER : POSX: % f\tPOSY: % f", blob_ptr->centroid.X, blob_ptr->centroid.Y);
#endif
          return true;
        };
        switch_ptr->timeStamp = millis();
      };
    }
    else {
      return false;
    };
  };
};

boolean toggle(llist_t* llist_ptr, tSwitch_t* switch_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > switch_ptr->posX - switch_ptr->rSize &&
        blob_ptr->centroid.X < switch_ptr->posX + switch_ptr->rSize) {
      if (blob_ptr->centroid.Y > switch_ptr->posY - switch_ptr->rSize &&
          blob_ptr->centroid.Y < switch_ptr->posY + switch_ptr->rSize) {
        if (millis() - switch_ptr->timeStamp > SWITCH_DEBOUNCE_TIMETCH) {
          switch_ptr->timeStamp = millis();
          switch_ptr->state = !switch_ptr->state;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_TOGGLE : STATE: % d", switch_ptr->state);
#endif
          return switch_ptr->state;
        };
        switch_ptr->timeStamp = millis();
      };
    };
  };
};

// Compute the grid index acording to the blob XY (centroid) coordinates
// Play corresponding midi **note** or **freq**
void gridPlay(llist_t* llist_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {                                                          // Test if the blob UID is less than MAX_SYNTH
      if (blob_ptr->state) {
        uint8_t keyPosX = (uint8_t)round((blob_ptr->centroid.X / (float)X_MAX) * GRID_COLS);  // Compute X window position
        uint8_t keyPosY = (uint8_t)round((blob_ptr->centroid.Y / (float)Y_MAX) * GRID_ROWS);  // Compute Y window position
        uint8_t keyPress = keyPosY * GRID_ROWS + keyPosX;                                     // Compute 1D key index position
        // Test if the blob is within the key limits
        if (!blob_ptr->lastState) {
#if HARDWARE_MIDI
          MIDI.sendNoteOn(midiKeyLayout[keyPress], 127, 1);                       // Send NoteON (CHANNEL_1)
#endif
#if DEBUG_MAPPING
          Serial.printf("\nGRID_GAP\tBLOB:%d\t\tKEYDOWN_NOTE_ON: %d",
                        blob_ptr->UID, midiKeyLayout[keyPress]);
#endif
          lastKeyPress[blob_ptr->UID] = keyPress;                                 // Save current key
        }
        else {
          if (keyPress != lastKeyPress[blob_ptr->UID]) {                          // Test if new key is pressed
#if HARDWARE_MIDI
            MIDI.sendNoteOff(midiKeyLayout[lastKeyPress[blob_ptr->UID]], 0, 1);   // Send NoteOFF (CHANNEL_1)
            MIDI.sendNoteOn(midiKeyLayout[keyPress], 127, 1);                     // Send NoteON (CHANNEL_1)
#endif
#if DEBUG_MAPPING
            Serial.printf("\nGRID_GAP\tBLOB: %d\tSLIDING_NOTE_OFF: %d\tSLIDING_NOTE_ON: %d",
                          blob_ptr->UID, midiKeyLayout[lastKeyPress[blob_ptr->UID]], midiKeyLayout[keyPress]);
#endif
            lastKeyPress[blob_ptr->UID] = keyPress;
          };
        };
      };
    }
    else {
#if HARDWARE_MIDI
      MIDI.sendNoteOff(midiKeyLayout[lastKeyPress[blob_ptr->UID]], 0, 1);         // Send NoteOFF (CHANNEL_1)
#endif
#if DEBUG_MAPPING
      Serial.printf("\nGRID_GAP\tBLOB:%d\tKEYUP_NOTE_OFF: %d",
                    blob_ptr->UID, midiKeyLayout[lastKeyPress[blob_ptr->UID]]);
#endif
    };
  };
};

// Pre-compute key min & max coordinates
void GRID_LAYOUT_SETUP() {
  int index = 0;
  //Serial.printf("\nDEBUG_KEY_SIZE: %f", KEY_SIZE);
  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      index = row * GRID_ROWS + col;
      keyPos[index].Xmin = col * KEY_SIZE + (col + 1) * GRID_GAP;
      keyPos[index].Xmax = (col + 1) * KEY_SIZE + (col + 1) * GRID_GAP;
      keyPos[index].Ymin = row * KEY_SIZE + (row + 1) * GRID_GAP;
      keyPos[index].Ymax = (row + 1) * KEY_SIZE + (row + 1) * GRID_GAP;
#if DEBUG_MAPPING
      Serial.printf("\nGRID_GAP\tXmin%d\tXmax%d\tYmin%d\tYmax%d",
                    keyPos[index].Xmin,
                    keyPos[index].Xmax,
                    keyPos[index].Ymin,
                    keyPos[index].Ymax
                   );
#endif
    };
  };
};

// Compute the grid index location acording to the blobs XY (centroid) coordinates
// Play corresponding midi **note** or **freq**
void gridGapPlay(llist_t* llist_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {                                                           // Test if the blob UID is less than MAX_SYNTH
      if (blob_ptr->state) {
        uint8_t keyPosX = (uint8_t)round((blob_ptr->centroid.X / (float)X_MAX) * GRID_COLS);   // Compute X window position
        uint8_t keyPosY = (uint8_t)round((blob_ptr->centroid.Y / (float)Y_MAX) * GRID_ROWS);   // Compute Y window position
        uint8_t keyPress = (keyPosY * GRID_COLS) + keyPosX;                                    // Compute 1D key index position
        // Test if the blob is within the key limits
        if (blob_ptr->centroid.X >= keyPos[keyPress].Xmin &&
            blob_ptr->centroid.X <= keyPos[keyPress].Xmax &&
            blob_ptr->centroid.Y >= keyPos[keyPress].Ymin &&
            blob_ptr->centroid.Y <= keyPos[keyPress].Ymax
           ) {
          if (!blob_ptr->lastState) {
#if HARDWARE_MIDI
            MIDI.sendNoteOn(keyMapp[keyPress], 127, 1);                             // Send NoteON (CHANNEL_1)
#endif
#if DEBUG_MAPPING
            Serial.printf("\nGRID_GAP\tBLOB:%d\t\tKEYDOWN_NOTE_ON: %d", blob_ptr->UID, keyPress);
#endif
            lastKeyPress[blob_ptr->UID] = keyPress;                                 // Save current key
          }
          else {
            if (keyPress != lastKeyPress[blob_ptr->UID]) {                          // Test if new key is pressed
#if HARDWARE_MIDI
              MIDI.sendNoteOff(keyMapp[lastKeyPress[blob_ptr->UID]], 0, 1);         // Send NoteOFF (CHANNEL_1)
              MIDI.sendNoteOn(keyMapp[keyPress], 127, 1);                           // Send NoteON (CHANNEL_1)
#endif
#if DEBUG_MAPPING
              Serial.printf("\nGRID_GAP\tBLOB: %d\tSLIDING_NOTE_OFF: %d\tSLIDING_NOTE_ON: %d", blob_ptr->UID, lastKeyPress[blob_ptr->UID], keyPress);
#endif
              lastKeyPress[blob_ptr->UID] = keyPress;
            };
          };
        };
      }
      else {
#if HARDWARE_MIDI
        MIDI.sendNoteOff(keyMapp[lastKeyPress[blob_ptr->UID]], 0, 1);               // Send NoteOFF (CHANNEL_1)
#endif
#if DEBUG_MAPPING
        Serial.printf("\nGRID_GAP\tBLOB:%d\tKEYUP_NOTE_OFF: %d", blob_ptr->UID, lastKeyPress[blob_ptr->UID]);
#endif
      };
    };
  };
};

void gridPopulate(llist_t* llist_ptr) {
  uint8_t keyIndex = 0;
  while (keyIndex < GRID_KEYS) {
    for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(llist_ptr); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
      midiKeyLayout[keyIndex].pithch = node_ptr->pithch;
      midiKeyLayout[keyIndex].velocity = node_ptr->velocity;
      midiKeyLayout[keyIndex].channel = node_ptr->channel;
      keyIndex++;
    }
  }
}

void vSlider(llist_t* llist_ptr, vSlider_t* slider_ptr) {
  uint8_t val = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
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
        }
      }
    }
  }
}

void hSlider(llist_t* llist_ptr, hSlider_t* slider_ptr) {
  uint8_t val = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
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
        }
      }
    }
  }
}

void cSlider(llist_t* llist_ptr, polar_t* polar_ptr, cSlider_t* slider_ptr) {
  float phi = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (int i = 0; i < C_SLIDERS; i++) {
      if (polar_ptr[blob_ptr->UID].r > slider_ptr[i].r - slider_ptr[i].width &&
          polar_ptr[blob_ptr->UID].r < slider_ptr[i].r + slider_ptr[i].width) {
        if (polar_ptr[blob_ptr->UID].phi > slider_ptr[i].phiOffset) {
          phi = polar_ptr[blob_ptr->UID].phi - slider_ptr[i].phiOffset;
        }
        else {
          phi = polar_ptr[blob_ptr->UID].phi + (PI2 - slider_ptr[i].phiOffset);
        }
#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_C_SLIDER_ % d phi : % f", i, map(constrain(phi, 0.2, 5.9), 0.2, 5.9, 0, 127));
#endif
      }
    }
  }
}
