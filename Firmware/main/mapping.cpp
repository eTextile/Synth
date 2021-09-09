/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

#if MAPPING_LAYAOUT

#define DEBOUNCE_TIME 15  // TO_REMOVE! this is done in find_blobs();

#define TRIGGERS 1
rect_t triggerKeys[TRIGGERS] = {0};
tSwitch_t triggerParams[TRIGGERS] = {
  {40, 30, 5, false}  // PARAMS[posX, posY, size, state]
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
        if (blob_ptr->centroid.X > triggerKeys[pos].Xmin &&
            blob_ptr->centroid.X < triggerKeys[pos].Xmax &&
            blob_ptr->centroid.Y > triggerKeys[pos].Ymin &&
            blob_ptr->centroid.Y < triggerKeys[pos].Ymax) {
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

#define TOGGLES 1
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
        if (blob_ptr->centroid.X > triggerKeys[pos].Xmin &&
            blob_ptr->centroid.X < triggerKeys[pos].Xmax &&
            blob_ptr->centroid.Y > triggerKeys[pos].Ymin &&
            blob_ptr->centroid.Y < triggerKeys[pos].Ymax) {
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

#define GRID_COLS              14
#define GRID_ROWS              10
#define GRID_KEYS              (GRID_COLS * GRID_ROWS)
#define GRID_GAP               1
#define KEY_SIZE_X             (uint8_t)((X_MAX - (GRID_GAP * (GRID_COLS + 1))) / GRID_COLS)
#define KEY_SIZE_Y             (uint8_t)((Y_MAX - (GRID_GAP * (GRID_ROWS + 1))) / GRID_ROWS)
#define GRID_X_SCALE_FACTOR    ((float)1/X_MAX) * GRID_COLS
#define GRID_Y_SCALE_FACTOR    ((float)1/Y_MAX) * GRID_ROWS

stroke_t key[GRID_KEYS] = {0};            // 1D array to store keys limits
stroke_t* lastKeyPress[MAX_SYNTH];        // 1D pointer array to store last keysPressed pointer

midiNode_t midiLayout[GRID_KEYS] = {0};   // 1D array to mapp MIDI notes with the grid layout

// Pre-compute key min & max coordinates
void GRID_LAYOUT_SETUP(void) {
  for (uint8_t row = 0; row < GRID_ROWS; row++) {
    uint8_t rowPos = row * GRID_COLS;
    for (uint8_t col = 0; col < GRID_COLS; col++) {
      uint8_t keyPress = rowPos + col;
      key[keyPress].rect.Xmin = col * KEY_SIZE_X + (col + 1) * GRID_GAP;
      key[keyPress].rect.Xmax = key[keyPress].rect.Xmin + KEY_SIZE_X;
      key[keyPress].rect.Ymin = row * KEY_SIZE_Y + (row + 1) * GRID_GAP;
      key[keyPress].rect.Ymax = key[keyPress].rect.Ymin + KEY_SIZE_Y;
#if DEBUG_MAPPING
      Serial.printf("\nGRID\tXmin:%d\tXmax:%d\tYmin:%d\tYmax:%d",
                    key[keyPress]->rect.Xmin,
                    key[keyPress]->rect.Xmax,
                    key[keyPress]->rect.Ymin,
                    key[keyPress]->rect.Ymax
                   );
#endif
    };
  };
};

// Compute the keyPresed position acording to the blobs XY (centroid) coordinates
// Add corresponding keys to the output MIDI liked list
void mapping_gridPlay(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {                                      // Test if the blob UID is less than MAX_SYNTH
      int keyPressX = round(blob_ptr->centroid.X * GRID_X_SCALE_FACTOR);  // Compute X grid position
      int keyPressY = round(blob_ptr->centroid.Y * GRID_Y_SCALE_FACTOR);  // Compute Y grid position
      int keyPress = keyPressY * GRID_COLS + keyPressX;                   // Compute 1D key index position
      if (!blob_ptr->lastState) lastKeyPress[blob_ptr->UID] = NULL;       // RAZ lastKeyPress when new blob is detected
      if (blob_ptr->state) {
        if (lastKeyPress[blob_ptr->UID] != &key[keyPress]) {
          // Test if the blob is within the key limits
          if (blob_ptr->centroid.X > key[keyPress].rect.Xmin &&
              blob_ptr->centroid.X < key[keyPress].rect.Xmax &&
              blob_ptr->centroid.Y > key[keyPress].rect.Ymin &&
              blob_ptr->centroid.Y < key[keyPress].rect.Ymax) {

            if (lastKeyPress[blob_ptr->UID] != NULL) { // Test
#if DEBUG_MAPPING
              Serial.printf("\nGRID\tBLOB:%d\t\tKEY_OFF:%p", blob_ptr->UID, &lastKeyPress[blob_ptr->UID]);
#else
              //MIDI.sendNoteOff(lastKeyPress[blob_ptr->UID]->val, 0, 1);           // Send NoteOFF (CHANNEL_1)
              midiNode_t* node = (midiNode_t*)llist_pop_front(&midi_node_stack);    // Get a node from the MIDI nodes stack
              node->pitch = lastKeyPress[blob_ptr->UID]->pitch;                     // Set the pitch
              node->velocity = 0;                                                   // Set the velocity
              //node->channel = blob_ptr->UID;                                      // Set the channel according to the blob ID
              node->channel = 1;                                                    // Set the channel to CHANNEL_1
              llist_push_front(&midiOut, node);                                     // Add the node to the midiOut linked liste
#endif
            };
#if DEBUG_MAPPING
            Serial.printf("\nGRID\tBLOB:%d\t\tKEY_DOWN:%p", blob_ptr->UID, &key[pos]);
#else
            //MIDI.sendNoteOn(midiLayout[blob_ptr->UID]->pithch, midiLayout[keyPress].velocity, 1);
            midiNode_t* node = (midiNode_t*)llist_pop_front(&midi_node_stack);    // Get a node from the MIDI nodes stack
            node->pitch = midiLayout[blob_ptr->UID].pitch;                        // Set the pitch
            node->velocity = midiLayout[keyPress].velocity;                       // Set the velocity
            //node->channel = blob_ptr->UID;                                      // Set the channel according to the blob ID
            node->channel = 1;                                                    // Set the channel to CHANNEL_1
            llist_push_front(&midiOut, node);                                     // Add the node to the midiOut linked liste
#endif
            lastKeyPress[blob_ptr->UID] = &key[keyPress];
          };
        };
      }
      else { // if !blob_ptr->state
#if DEBUG_MAPPING
        Serial.printf("\nGRID\tBLOB:%d\t\tKEY_UP:%p", blob_ptr->UID, &key[pos]);
#else
        //MIDI.sendNoteOff(lastKeyPress[blob_ptr->UID]->val, 0, 1);           // Send NoteOFF (CHANNEL_1)
        midiNode_t* node = (midiNode_t*)llist_pop_front(&midi_node_stack);    // Get a node from the MIDI nodes stack
        node->pitch = lastKeyPress[blob_ptr->UID]->pitch;                     // Set the pitch
        node->velocity = 0;                                                   // Set the velocity to 0
        //node->channel = blob_ptr->UID;                                      // Set the channel according to the blob ID
        node->channel = 1;                                                    // Set the channel to CHANNEL_1
        llist_push_front(&midiOut, node);                                     // Add the node to the midiOut linked liste
#endif
      };
    };
  };
};

void mapping_gridPopulate(void) {
  uint8_t key = 0;
  while (key < GRID_KEYS) {
    for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiIn); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
      midiLayout[key].pitch = node_ptr->pitch;
      midiLayout[key].velocity = node_ptr->velocity;
      midiLayout[key].channel = node_ptr->channel;
      key++;
    };
  };
};

#define VSLIDERS 2
rect_t vSlider[VSLIDERS] = {0};
vSlider_t vSliderParams[VSLIDERS] = {
  {10, 10, 40, 10, 0},  // ARGS[posX, Ymin, Ymax, width, val]
  {30, 10, 40, 10, 0}   // ARGS[posX, Ymin, Ymax, width, val]
};

void VSLIDERS_SETUP(void) {
  for (uint8_t pos = 0; pos < VSLIDERS; pos++) {
    vSlider[pos].Xmin = vSliderParams[pos].posX - (vSliderParams[pos].width / 2);
    vSlider[pos].Xmax = vSliderParams[pos].posX + (vSliderParams[pos].width / 2);
    vSlider[pos].Ymin = vSliderParams[pos].Ymin;
    vSlider[pos].Ymax = vSliderParams[pos].Ymax;
  };
};

void mapping_vSliders(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t pos = 0; pos < VSLIDERS; pos++) {
      if (blob_ptr->centroid.X > vSlider[pos].Xmin &&
          blob_ptr->centroid.X < vSlider[pos].Xmax &&
          blob_ptr->centroid.Y > vSlider[pos].Ymin &&
          blob_ptr->centroid.Y < vSlider[pos].Ymax) {
        uint8_t val = round(map(blob_ptr->centroid.Y, vSlider[pos].Ymin, vSlider[pos].Ymax, 0, 127)); // [0:127]
        if (val != vSliderParams[pos].val) {
          vSliderParams[pos].val = val;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_VSLIDER:\t%d", val);
#endif
        };
        break;
      };
    };
  };
};

#define HSLIDERS 2
rect_t hSlider[HSLIDERS] = {0};
hSlider_t hSliderParams[HSLIDERS] = {
  {15, 15, 40, 10, 0},  // ARGS[posY, Xmin, Xmax, height, val]
  {30, 15, 40, 10, 0}   // ARGS[posY, Xmin, Xmax, height, val]
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
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t pos = 0; pos < HSLIDERS; pos++) {
      if (blob_ptr->centroid.X > hSlider[pos].Xmin &&
          blob_ptr->centroid.X < hSlider[pos].Xmax &&
          blob_ptr->centroid.Y > hSlider[pos].Ymin &&
          blob_ptr->centroid.Y < hSlider[pos].Ymax) {
        uint8_t val = round(map(blob_ptr->centroid.X, hSlider[pos].Xmin, hSlider[pos].Xmax, 0, 127)); // [0:127]
        if (val != hSliderParams[pos].val) {
          hSliderParams[pos].val = val;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_HSLIDER:\t%d", val);
#endif
        };
        break;
      };
    };
  };
};

// https://live.staticflickr.com/65535/50866229007_398065fd9a_k_d.jpg
// CIRCULAR_SLIDERS_CONSTANTS
#define CS_TRACKS         4
#define CS_SLIDERS        7
#define CS_RADIUS         (float)(X_MAX - X_MIN) / 2
#define CS_MARGIN         (float)0.5
#define CS_RMIN           (float)3
#define CS_RMAX           (float)(CS_RADIUS - CS_MARGIN)
#define CS_TRACK_WIDTH    (float)((CS_RMAX - CS_RMIN) / CS_TRACKS)
#define CS_SCALE_FACTOR   (float)(1 / CS_TRACK_WIDTH)

cTrack_t cTrack[CS_TRACKS] = {
  {1, 0, 0  },        // PARAMS[sliders, index, track-offset-rad]
  {1, 1, 1  },        // PARAMS[sliders, index, track-offset-rad]
  {2, 2, 2.4},        // PARAMS[sliders, index, track-offset-rad]
  {3, 4, 3.0}         // PARAMS[sliders, index, track-offset-rad]
};

cSlider_t cSliders[CS_SLIDERS] = {
  {0,    IIPi, 0},  // PARAMS[thetaMin, thetaMax, val]
  {0,    IIPi, 0},  // PARAMS[thetaMin, thetaMax, val]
  {0,    3.60, 0},  // PARAMS[thetaMin, thetaMax, val]
  {3.80, IIPi, 0},  // PARAMS[thetaMin, thetaMax, val]
  {0,    3.20, 0},  // PARAMS[thetaMin, thetaMax, val]
  {3.40, 5.20, 0},  // PARAMS[thetaMin, thetaMax, val]
  {5.30, 8, IIPi}   // PARAMS[thetaMin, thetaMax, val]
};

cSlider_t* csMapping[CS_SLIDERS] = {NULL};

void CSLIDERS_SETUP(void) {
  //Serial.printf("\nDEBUG_CSLIDER_SETUP\tCS_RADIUS:\t%f", CS_RADIUS);
  //Serial.printf("\nDEBUG_CSLIDER_SETUP\tCS_RMAX:\t%f", CS_RMAX);
  //Serial.printf("\nDEBUG_CSLIDER_SETUP\tCS_WIDTH:\t%f", CS_TRACK_WIDTH);
  //Serial.printf("\nDEBUG_CSLIDER_SETUP\tCS_SCALE_FACTOR:\t%f", CS_SCALE_FACTOR);
};

void mapping_cSliders(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    float x = blob_ptr->centroid.X - CS_RADIUS;
    float y = blob_ptr->centroid.Y - CS_RADIUS;
    float radius = sqrt(x * x + y * y);
    if (radius > CS_RMIN && radius < CS_RMAX) {
      float theta = 0;
      uint8_t track = (uint8_t)round(radius * CS_SCALE_FACTOR) - 1; // Compute track position
      // Rotation of Axes through an angle without shifting Origin
      float posX = x * cos(cTrack[track].offset) + y * sin(cTrack[track].offset);
      float posY = -x * sin(cTrack[track].offset) + y * cos(cTrack[track].offset);
      if (posX == 0 && 0 < posY) {
        theta = PiII;
      } else if (posX == 0 && posY < 0) {
        theta = IIIPiII;
      } else if (posX < 0) {
        theta = atanf(posY / posX) + PI;
      } else if (posY < 0) {
        theta = atanf(posY / posX) + IIPi;
      } else {
        theta = atanf(posY / posX);
      }
#if DEBUG_MAPPING
      Serial.printf("\nDEBUG_CSLIDER:\tTrack:\t%d\tTheta:\t%f", track, theta);
#endif
      if (blob_ptr->state) {
        if (!blob_ptr->lastState) {
          for (uint8_t id = cTrack[track].index; id < cTrack[track].index + cTrack[track].sliders; id++) {
            if (theta > cSliders[id].thetaMin && theta < cSliders[id].thetaMax) {
              csMapping[blob_ptr->UID] = &cSliders[id]; // Record pointer to slider
#if DEBUG_MAPPING
              //Serial.printf("\nDEBUG_CSLIDER:\tBlob:\t%d\tSlider:\t%d", blob_ptr->UID, id);
#else
#endif
            };
          };
        }
        else {
          cSlider_t* cSlider_ptr = csMapping[blob_ptr->UID];
          if (cSlider_ptr != NULL) {
            if (theta > cSlider_ptr->thetaMin && theta < cSlider_ptr->thetaMax) {
              cSlider_ptr->val = (int8_t)map(theta, cSlider_ptr->thetaMin, cSlider_ptr->thetaMax, 0, 127);
#if DEBUG_MAPPING
              //Serial.printf("\nDEBUG_CSLIDER:\tRadius:\t%f\tTheta:\t%f\tVal:\t%d", radius, theta, cSlider_ptr->val);
#else
#endif
            };
          };
        };
      }
      else {
        csMapping[blob_ptr->UID] = NULL;
      };
    };
  };
};

#define CCHANGE 2
cChange_t cChange[CCHANGE] = {
  {NULL, BD, 44, 1, 0},  // ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]
  {NULL, BD, 44, 1, 0}   // ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]
};

void mapping_cChange(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (int index = 0; index < CCHANGE; index++) {
      if (blob_ptr->UID == cChange[index].blobID) {
        // Test if the blob is alive
        if (blob_ptr->state) {
          switch (cChange[index].mappVal) {
            case BX:
              if (blob_ptr->centroid.X != cChange[index].val) {
                cChange[index].val = blob_ptr->centroid.X;
#if DEBUG_MIDI_HARDWARE
                Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.X, 0, 127), cChange[index].midiChannel);
#else
                //MIDI.sendControlChange(cChange[index].cChange, constrain(blob_ptr->centroid.X, 0, 127), cChange[index].midiChannel);
#endif
              };
              break;
            case BY:
              if (blob_ptr->centroid.Y != cChange[index].val) {
                cChange[index].val = blob_ptr->centroid.Y;
#if DEBUG_MIDI_HARDWARE
                Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.Y, 0, 127), ccParams_ptr->midiChannel);
#else
                //MIDI.sendControlChange(cChange[index].cChange, constrain(blob_ptr->centroid.Y, 0, 127), cChange[index].midiChannel);
#endif
              };
              break;
            case BW:
              if (blob_ptr->box.W != cChange[index].val) {
                cChange[index].val = blob_ptr->box.W;
#if DEBUG_MIDI_HARDWARE
                Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.W, 0, 127), ccParams_ptr->midiChannel);
#else
                //MIDI.sendControlChange(cChange[index].cChange, constrain(blob_ptr->box.W, 0, 127), cChange[index].midiChannel);
#endif
              };
              break;
            case BH:
              if (blob_ptr->box.H != cChange[index].val) {
                cChange[index].val = blob_ptr->box.H;
#if DEBUG_MIDI_HARDWARE
                Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.H, 0, 127), cChange_t->midiChannel);
#else
                //MIDI.sendControlChange(cChange[index].cChange, constrain(blob_ptr->box.H, 0, 127), cChange[index].midiChannel);
#endif
              };
              break;
            case BD:
              if (blob_ptr->centroid.Z != cChange[index].val) {
                cChange[index].val = blob_ptr->centroid.Z;
#if DEBUG_MIDI_HARDWARE
                Serial.printf("\nBLOB:%d\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, cChange[index].cChange, constrain(blob_ptr->centroid.Z, 0, 127), cChange[index].midiChannel);
#else
                //MIDI.sendControlChange(cChange[index].cChange, constrain(blob_ptr->centroid.Z, 0, 127), cChange[index].midiChannel);
#endif
              };
              break;
          };
        };
      };
    };
  };
};
#endif
