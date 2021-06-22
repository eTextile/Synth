/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

#if MAPPING_LAYAOUT

#define DEBOUNCE_TIME  15  // TO_REMOVE! this is done in find_blobs();

#define TRIGGERS   1
rect_t triggerKeys[TRIGGERS] = {0};
tSwitch_t triggerParams[TRIGGERS] = {
  {40, 30, 5, false}  // ARGS[posX, posY, size, state]
};

void TRIGGERS_SETUP(void) {
  for (uint8_t pos = 0; pos < TRIGGERS; pos++) {
    triggerKeys[pos].Xmin = triggerParams[pos].posX - triggerParams[pos].size;
    triggerKeys[pos].Xmax = triggerParams[pos].posX + triggerParams[pos].size;
    triggerKeys[pos].Ymin = triggerParams[pos].posY - triggerParams[pos].size;
    triggerKeys[pos].Ymax = triggerParams[pos].posY + triggerParams[pos].size;
  };
};

tSwitch_t* mapping_triggers(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {                                          // Test if the blob UID is less than MAX_SYNTH
      for (uint8_t pos = 0; pos < TRIGGERS; pos++) {
        if (blob_ptr->centroid.X > triggerKeys[pos].Xmin && blob_ptr->centroid.X < triggerKeys[pos].Xmax ) {
          if (blob_ptr->centroid.Y > triggerKeys[pos].Ymin && blob_ptr->centroid.Y < triggerKeys[pos].Ymax) {
            if (!blob_ptr->lastState) {
              triggerParams[pos].state = true;
              return &triggerParams[pos];
#if DEBUG_MAPPING
              //Serial.printf("\nDEBUG_TRIGGER:\tPOSX:%f\tPOSY:%f", blob_ptr->centroid.X, blob_ptr->centroid.Y);
              Serial.printf("\nDEBUG_TRIGGER:\ttrigger_ptr:%p", &triggerParams[pos]);
#endif
            }
            else {
              triggerParams[pos].state = false;
              return NULL;
            };
          };
        };
      }
    };
  };
};

#define TOGGLES   1
rect_t toggleKeys[TOGGLES] = {0};
tSwitch_t toggleParams[TOGGLES] = {
  {10, 10, 5, false}  // ARGS[posX, posY, size, state]
};

void TOGGLES_SETUP(void) {
  for (uint8_t pos = 0; pos < TOGGLES; pos++) {
    toggleKeys[pos].Xmin = toggleParams[pos].posX - toggleParams[pos].size;
    toggleKeys[pos].Xmax = toggleParams[pos].posX + toggleParams[pos].size;
    toggleKeys[pos].Ymin = toggleParams[pos].posY - toggleParams[pos].size;
    toggleKeys[pos].Ymax = toggleParams[pos].posY + toggleParams[pos].size;
  };
};

tSwitch_t* mapping_toggles(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {   // Test if the blob UID is less than MAX_SYNTH
      for (uint8_t pos = 0; pos < TOGGLES; pos++) {
        if (blob_ptr->centroid.X > triggerKeys[pos].Xmin && blob_ptr->centroid.X < triggerKeys[pos].Xmax ) {
          if (blob_ptr->centroid.Y > triggerKeys[pos].Ymin && blob_ptr->centroid.Y < triggerKeys[pos].Ymax) {
            if (!blob_ptr->lastState) {
              toggleParams[pos].state = !toggleParams[pos].state;
              return &triggerParams[pos];
#if DEBUG_MAPPING
              //Serial.printf("\nDEBUG_TOGGLE:\tPOSX:%f\tPOSY:%f", blob_ptr->centroid.X, blob_ptr->centroid.Y);
              Serial.printf("\nDEBUG_TRIGGER:\ttoggle_ptr:%p", &triggerParams[pos]);
#endif
            };
          };
        };
      };
    };
  };
};

#define GRID_COLS              14
#define GRID_ROWS              10
#define GRID_KEYS              (GRID_COLS * GRID_ROWS)
#define GRID_GAP               1
#define KEY_SIZE_X             (uint8_t)((X_MAX - (GRID_GAP * (GRID_COLS + 1))) / GRID_COLS)
#define KEY_SIZE_Y             (uint8_t)((Y_MAX - (GRID_GAP * (GRID_ROWS + 1))) / GRID_ROWS)
#define GRID_X_SCALE_FACTOR    (float)(GRID_COLS * (1/X_MAX))
#define GRID_Y_SCALE_FACTOR    (float)(GRID_ROWS * (1/Y_MAX))

rect_t keys[GRID_KEYS] = {0};          // 1D array to store keys limits
rect_t* keyPress_ptr[MAX_SYNTH];       // 1D pointer array to store last keysPressed pointer
rect_t* lastKeyPress_ptr[MAX_SYNTH];   // 1D pointer array to store last keysPressed pointer

midiNode_t midiLayout[GRID_KEYS] = {0};  // 1D array to mapp incoming midi notes in the grid layout

// Pre-compute key min & max coordinates
void GRID_LAYOUT_SETUP(void) {
  for (uint8_t row = 0; row < GRID_ROWS; row++) {
    uint8_t rowPos = row * GRID_COLS;
    for (uint8_t col = 0; col < GRID_COLS; col++) {
      uint8_t pos = rowPos + col;
      keys[pos].Xmin = col * KEY_SIZE_X + (col + 1) * GRID_GAP;
      keys[pos].Xmax = keys[pos].Xmin + KEY_SIZE_X;
      keys[pos].Ymin = row * KEY_SIZE_Y + (row + 1) * GRID_GAP;
      keys[pos].Ymax = keys[pos].Ymin + KEY_SIZE_Y;
#if DEBUG_MAPPING
      Serial.printf("\nGRID\tXmin:%d\tXmax:%d\tYmin:%d\tYmax:%d", *key_ptr->Xmin, *key_ptr->Xmax, *key_ptr->Ymin, *key_ptr->Ymax);
#endif
    };
  };
};

// Compute the grid index location acording to the blobs XY (centroid) coordinates
// Play corresponding midi **note** or **freq**
void mapping_gridPlay(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {                                          // Test if the blob UID is less than MAX_SYNTH
      int keyPosX = round(blob_ptr->centroid.X * GRID_X_SCALE_FACTOR); // Compute X grid position
      int keyPosY = round(blob_ptr->centroid.Y * GRID_Y_SCALE_FACTOR); // Compute Y grid position
      int pos = keyPosY * GRID_COLS + keyPosX;                      // Compute 1D key index position
      if (!blob_ptr->lastState) lastKeyPress_ptr[blob_ptr->UID] = NULL;
      if (blob_ptr->state) {
        if (&keys[pos] != lastKeyPress_ptr[blob_ptr->UID]) {
          // Test if the blob is within the key limits
          if (blob_ptr->centroid.X > keys[pos].Xmin && blob_ptr->centroid.X < keys[pos].Xmax &&
              blob_ptr->centroid.Y > keys[pos].Ymin && blob_ptr->centroid.Y < keys[pos].Ymax) {
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
            Serial.printf("\nGRID\tBLOB:%d\t\tKEY_DOWN:%d", blob_ptr->UID, keys[pos].val);
#else
            //MIDI.sendNoteOn(keyPress_ptr->val, 127, 1); // Send NoteON (CHANNEL_1)
#endif
            lastKeyPress_ptr[blob_ptr->UID] = &keys[pos];
          };
        };
      }
      else {
#if DEBUG_MAPPING
        Serial.printf("\nGRID\tBLOB:%d\t\tKEY_UP:%d", blob_ptr->UID, keys[pos].val);
#else
        //MIDI.sendNoteOff(lastKeyPress_ptr[blob_ptr->UID]->val, 0, 1);  // Send NoteOFF (CHANNEL_1)
#endif
      };
    };
  };
};

void mapping_gridPopulate(void) {
  uint8_t key = 0;
  while (key < GRID_KEYS) {
    for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiIn); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
      midiLayout[key].pithch = node_ptr->pithch;
      midiLayout[key].velocity = node_ptr->velocity;
      midiLayout[key].channel = node_ptr->channel;
      key++;
    };
  };
};

#define VSLIDERS   2
rect_t vSlider[VSLIDERS] = {0};
vSlider_t vSliderParams[VSLIDERS] = {
  {10, 15, 40, 5, 0},  // ARGS[posX, Ymin, Ymax, width, val]
  {10, 15, 40, 5, 0}   // ARGS[posX, Ymin, Ymax, width, val]
};

void VSLIDERS_SETUP(void) {
  for (uint8_t pos = 0; pos < VSLIDERS; pos++) {
    vSlider[pos].Xmin = vSliderParams[pos].posX - vSliderParams[pos].width / 2;
    vSlider[pos].Xmax = vSliderParams[pos].posX + vSliderParams[pos].width / 2;
    vSlider[pos].Ymin = vSliderParams[pos].Ymin;
    vSlider[pos].Ymax = vSliderParams[pos].Ymax;
  };
};

void mapping_vSliders(void) {
  uint8_t val = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t pos = 0; pos < VSLIDERS; pos++) {
      if (blob_ptr->centroid.X > vSlider[pos].Xmin && blob_ptr->centroid.X < vSlider[pos].Xmax) {
        if (blob_ptr->centroid.Y > vSlider[pos].Ymin && blob_ptr->centroid.Y < vSlider[pos].Ymax) {
          val = round(map(blob_ptr->centroid.Y, vSlider[pos].Ymin, vSlider[pos].Ymax, 0, 127)); // [0:127]
          if (val != vSliderParams[pos].val) {
            vSliderParams[pos].val = val;
#if DEBUG_MAPPING
            Serial.printf("\nDEBUG_V_SLIDER : % d", val);
#endif
          };
        };
      };
    };
  };
};

#define HSLIDERS   2
rect_t hSlider[HSLIDERS] = {0};
hSlider_t hSliderParams[HSLIDERS] = {
  {10, 15, 40, 5, 0},  // ARGS[posY, Xmin, Xmax, height, val]
  {10, 15, 40, 5, 0}   // ARGS[posY, Xmin, Xmax, height, val]
};

void HSLIDERS_SETUP(void) {
  for (uint8_t pos = 0; pos < VSLIDERS; pos++) {
    hSlider[pos].Xmin = hSliderParams[pos].Xmin;
    hSlider[pos].Xmax = hSliderParams[pos].Xmax;
    hSlider[pos].Ymin = hSliderParams[pos].posY - hSliderParams[pos].height / 2;
    hSlider[pos].Ymax = hSliderParams[pos].posY + hSliderParams[pos].height / 2;
  };
};

void mapping_hSliders(void) {
  uint8_t val = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t pos = 0; pos < HSLIDERS; pos++) {
      if (blob_ptr->centroid.X > hSlider[pos].Xmin && blob_ptr->centroid.X < hSlider[pos].Xmax) {
        if (blob_ptr->centroid.Y > hSlider[pos].Ymin && blob_ptr->centroid.Y < hSlider[pos].Ymax) {
          val = round(map(blob_ptr->centroid.X, hSlider[pos].Xmin, hSlider[pos].Xmax, 0, 127)); // [0:127]
          if (val != hSliderParams[pos].val) {
            hSliderParams[pos].val = val;
#if DEBUG_MAPPING
            Serial.printf("\nDEBUG_H_SLIDER : % d", val);
#endif
          };
        };
      };
    };
  };
};

// CIRCULAR_SLIDERS_CONSTANTS
#define CS_ID            6
#define CS_TRACKS        4
#define CS_WIDTH         5
#define CS_GAP           2
#define CS_MARGIN_IN     4
#define CS_MARGIN_OUT    4
#define CS_SCALE_FACTOR  (float)((CS_TRACKS * (1 / ((Y_MAX / 2) - (CS_MARGIN_IN + CS_MARGIN_OUT))))) // SCAL FACTOR

cTrack_t cTracks[CS_TRACKS] = {
  { 0, 0, 0},  // ARGS[div, rMin, rMax]
  { 2, 0, 0},  // ARGS[div, rMin, rMax]
  { 1, 0, 0},  // ARGS[div, rMin, rMax]
  { 3, 0, 0}   // ARGS[div, rMin, rMax]
};

cSlider_t cSliders[CS_ID] = {
  { 0, 3.8,  5, 0 },  // ARGS[phiMin, phiMax, phiOffset, val]
  { 0, 3.8, 10, 0 },  // ARGS[phiMin, phiMax, phiOffset, val]
  { 0, 4.8,  5, 0 },  // ARGS[phiMin, phiMax, phiOffset, val]
  { 0, 4.8,  5, 0 },  // ARGS[phiMin, phiMax, phiOffset, val]
  { 0, 4.8,  5, 0 },  // ARGS[phiMin, phiMax, phiOffset, val]
  { 0, 4.8,  5, 0 }   // ARGS[phiMin, phiMax, phiOffset, val]
};

void C_SLIDER_SETUP(void) {
  for (uint8_t track = 0; track < CS_TRACKS; track++) {
    cTracks[track].rMin = (CS_TRACKS * (CS_WIDTH + CS_GAP)) + (CS_WIDTH / 2);
    cTracks[track].rMax = (CS_TRACKS * (CS_WIDTH + CS_GAP)) - (CS_WIDTH / 2);
  };
};

void mapping_cSliders(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {                              // Test if the blob UID is less than MAX_SYNTH
      uint8_t track = (uint8_t)round(blob_ptr->polar.r * CS_SCALE_FACTOR); // Compute track position
      if (blob_ptr->polar.r > cTracks[track].rMin &&
          blob_ptr->polar.r < cTracks[track].rMax) {
        for (uint8_t pos = 0; pos < cTracks[track].div; pos++) {

          if (blob_ptr->polar.phi > cSliders[pos].phiMin && blob_ptr->polar.phi < cSliders[pos].phiMax) {

            //cSliders[pos].val = blob_ptr->polar.phi - cSliders[pos].phiOffset; // TODO! mapping 0-127

          }
          else {
            //float phi = blob_ptr->polar.phi + (PI2 - cSliders_ptr[pos].phiOffset);
          };
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_C_SLIDER_%d - phi:%f", pos, cSliders_ptr[pos].val );
#endif
        };
      };
    };
  };
};

// cChange_t -> ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]
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
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.H, 0, 127), cChange_t->midiChannel);
#else
              //MIDI.sendControlChange(cChange_ptr->cChange, constrain(blob_ptr->box.H, 0, 127), cChange_ptr->midiChannel);
#endif
            };
            break;
          case BD:
            if (blob_ptr->centroid.Z != cChange_ptr->val) {
              cChange_ptr->val = blob_ptr->centroid.Z;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nBLOB:%d\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, cChange_ptr->cChange, constrain(blob_ptr->centroid.Z, 0, 127), cChange_ptr->midiChannel);
#else
              //MIDI.sendControlChange(cChange_ptr->cChange, constrain(blob_ptr->centroid.Z, 0, 127), cChange_ptr->midiChannel);
#endif
            };
            break;
        };
      };
    };
  };
};
#endif
