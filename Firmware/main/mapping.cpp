/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

#define TRIGGERS 2

stroke_t triggerParam[TRIGGERS] = {
  {30, 20, 10, 44},  // PARAMS[posX, posY, size, note]
  {50, 30, 10, 33}   // PARAMS[posX, posY, size, note]
};

tSwitch_t triggerKey[TRIGGERS] = {0}; // ARGS[[rect.Xmin, rect.Xmax, rect.Ymin, rect.Ymax], state]

void TRIGGER_SETUP(void) {
  for (uint8_t keyPos = 0; keyPos < TRIGGERS; keyPos++) {
    triggerKey[keyPos].rect.Xmin = triggerParam[keyPos].posX - round(triggerParam[keyPos].size / 2);
    triggerKey[keyPos].rect.Xmax = triggerParam[keyPos].posX + round(triggerParam[keyPos].size / 2);
    triggerKey[keyPos].rect.Ymin = triggerParam[keyPos].posY - round(triggerParam[keyPos].size / 2);
    triggerKey[keyPos].rect.Ymax = triggerParam[keyPos].posY + round(triggerParam[keyPos].size / 2);
  };
};

void mapping_trigger(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {                                          // Test if the blob UID is less than MAX_SYNTH
      for (uint8_t keyPos = 0; keyPos < TRIGGERS; keyPos++) {
        // Test if the blob is within the key limits
        if (blob_ptr->centroid.X > triggerKey[keyPos].rect.Xmin &&
            blob_ptr->centroid.X < triggerKey[keyPos].rect.Xmax &&
            blob_ptr->centroid.Y > triggerKey[keyPos].rect.Ymin &&
            blob_ptr->centroid.Y < triggerKey[keyPos].rect.Ymax) {
          if (!blob_ptr->lastState) {
            midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
            node_ptr->midiMsg.status = midi::NoteOn;                                // Set MIDI message status to NOTE_OFF
            node_ptr->midiMsg.data1 = triggerParam[keyPos].note;                    // Set the note
            //node_ptr->midiMsg.data2 = blob_ptr->velocity.Z                        // Set the velocity TODO
            node_ptr->midiMsg.data2 = 127;                                          // Set the velocity
            node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                        // Set the channel see config.h
            llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#if DEBUG_MAPPING
            Serial.printf("\nDEBUG_TRIGGER:\tNOTE_ON : %d", triggerParam[keyPos].note);
#endif
          }
          else if (!blob_ptr->state) {
            midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
            node_ptr->midiMsg.status = midi::NoteOff;                               // Set MIDI message status to NOTE_OFF
            node_ptr->midiMsg.data1 = triggerParam[keyPos].note;                    // Set the note
            node_ptr->midiMsg.data2 = 0;                                            // Set the velocity
            node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                        // Set the channel see config.h
            llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#if DEBUG_MAPPING
            Serial.printf("\nDEBUG_TRIGGER:\tNOTE_OFF : %d", triggerParam[keyPos].note);
#endif
          };
        };
      };
    }
  };
};

#define TOGGLES 2

stroke_t toggleParam[TOGGLES] = {
  {10, 10, 5, 64},  // ARGS[posX, posY, size, note]
  {20, 10, 5, 65}   // ARGS[posX, posY, size, note]
};

tSwitch_t toggleKey[TOGGLES] = {0};  // ARGS[[rect.Xmin, rect.Xmax, rect.Ymin, rect.Ymax], state]

void TOGGLE_SETUP(void) {
  for (uint8_t keyPos = 0; keyPos < TOGGLES; keyPos++) {
    toggleKey[keyPos].rect.Xmin = toggleParam[keyPos].posX - round(toggleParam[keyPos].size / 2);
    toggleKey[keyPos].rect.Xmax = toggleParam[keyPos].posX + round(toggleParam[keyPos].size / 2);
    toggleKey[keyPos].rect.Ymin = toggleParam[keyPos].posY - round(toggleParam[keyPos].size / 2);
    toggleKey[keyPos].rect.Ymax = toggleParam[keyPos].posY + round(toggleParam[keyPos].size / 2);
    toggleKey[keyPos].state = false;
  };
};

void mapping_toggle(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {   // Test if the blob UID is less than MAX_SYNTH
      for (uint8_t keyPos = 0; keyPos < TOGGLES; keyPos++) {
        // Test if the blob is within the key limits
        if (blob_ptr->centroid.X > toggleKey[keyPos].rect.Xmin &&
            blob_ptr->centroid.X < toggleKey[keyPos].rect.Xmax &&
            blob_ptr->centroid.Y > toggleKey[keyPos].rect.Ymin &&
            blob_ptr->centroid.Y < toggleKey[keyPos].rect.Ymax) {
          if (!blob_ptr->lastState) {
            toggleKey[keyPos].state = !toggleKey[keyPos].state;
            midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);    // Get a node from the MIDI node stack
            if (toggleKey[keyPos].state) {
#if DEBUG_MAPPING
              Serial.printf("\nDEBUG_TRIGGER:\tNOTE_ON : %d", toggleParam[keyPos].note);
#else
              node_ptr->midiMsg.status = midi::NoteOn;                                // Set MIDI message status to NOTE_OFF
              node_ptr->midiMsg.data1 = toggleParam[keyPos].note;                     // Set the note
              //node_ptr->midiMsg.data2 = blob_ptr->velocity.Z                        // Set the velocity
              node_ptr->midiMsg.data2 = 127;                                          // Set the velocity
              node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                        // Set the channel see config.h
              llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
            } else {
#if DEBUG_MAPPING
              Serial.printf("\nDEBUG_TRIGGER:\tNOTE_OFF : %d", toggleParam[keyPos].note);
#else
              node_ptr->midiMsg.status = midi::NoteOff;                               // Set MIDI message status to NOTE_OFF
              node_ptr->midiMsg.data1 = toggleParam[keyPos].note;                     // Set the note
              node_ptr->midiMsg.data2 = 0;                                            // Set the velocity
              node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                        // Set the channel see config.h
              llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
            };
          };
        };
      };
    };
  };
};

#define GRID_GAP               (float)0.4
#define KEY_SIZE_X             (float)((X_MAX - (GRID_GAP * (GRID_COLS + 1))) / GRID_COLS)
#define KEY_SIZE_Y             (float)((Y_MAX - (GRID_GAP * (GRID_ROWS + 1))) / GRID_ROWS)
#define GRID_X_SCALE_FACTOR    ((float)1/X_MAX) * GRID_COLS
#define GRID_Y_SCALE_FACTOR    ((float)1/Y_MAX) * GRID_ROWS

tSwitch_t key[GRID_KEYS] = {0};               // ARGS[posX, Ymin, Ymax, width, state] : 1D array to store keys limits & state
int8_t lastKeyPress[MAX_SYNTH] = {NULL};      // 1D array to store last keys pressed value

unsigned long int lastTime[MAX_SYNTH] = {0};  // 1D array to store last keys pressed value

// Pre-compute key min & max coordinates
void GRID_LAYOUT_SETUP(void) {
  for (uint8_t row = 0; row < GRID_ROWS; row++) {
    uint8_t rowPos = row * GRID_COLS;
    for (uint8_t col = 0; col < GRID_COLS; col++) {
      uint8_t keyPos = rowPos + col;
      key[keyPos].rect.Xmin = col * KEY_SIZE_X + (col + 1) * GRID_GAP;
      key[keyPos].rect.Xmax = key[keyPos].rect.Xmin + KEY_SIZE_X;
      key[keyPos].rect.Ymin = row * KEY_SIZE_Y + (row + 1) * GRID_GAP;
      key[keyPos].rect.Ymax = key[keyPos].rect.Ymin + KEY_SIZE_Y;
#if DEBUG_MAPPING
      Serial.printf("\nGRID\tkey:%d\t_Xmin:%f \t_Xmax:%f \t_Ymin:%f \t_Ymax:%f",
                    keyPos,
                    key[keyPos].rect.Xmin,
                    key[keyPos].rect.Xmax,
                    key[keyPos].rect.Ymin,
                    key[keyPos].rect.Ymax
                   );
#endif
    };
  };
};

// Compute the keyPresed position acording to the blobs XY (centroid) coordinates
// Add corresponding MIDI message to the MIDI out liked list
void mapping_grid_update(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {                                                // Test if the blob UID is less than MAX_SYNTH
      uint8_t keyPressX = lround(blob_ptr->centroid.X * GRID_X_SCALE_FACTOR);       // Compute X grid position
      uint8_t keyPressY = lround(blob_ptr->centroid.Y * GRID_Y_SCALE_FACTOR);       // Compute Y grid position
      uint8_t keyPress = keyPressY * GRID_COLS + keyPressX;                         // Compute 1D key index position
      //Serial.printf("\nGRID\tKEY:%d\tPOS_X:%d\tPOS_Y:%d", keyPress, keyPressX, keyPressY);
      //Serial.printf("\nGRID\tBLOB:%d\tBLOB_X:%f\tBLOB_Y:%f", blob_ptr->UID, blob_ptr->centroid.X, blob_ptr->centroid.X);

      if (blob_ptr->state) {                                                        // Test if the blob is alive
        if (gridLayout[keyPress] != lastKeyPress[blob_ptr->UID]) {                  // Test if the blob is touching a new key
          /*
            // Test if the blob is within the key limits <-- This is too slow!
            if (blob_ptr->centroid.X > key[keyPress].rect.Xmin &&
              blob_ptr->centroid.X < key[keyPress].rect.Xmax &&
              blob_ptr->centroid.Y > key[keyPress].rect.Ymin &&
              blob_ptr->centroid.Y < key[keyPress].rect.Ymax) {
          */
          if (lastKeyPress[blob_ptr->UID] != -1) {                                  // Test if the blob was touching another key

#if DEBUG_MAPPING
            Serial.printf("\nGRID\tBLOB:%d\t\tKEY_SLIDING_OFF:%d", blob_ptr->UID, lastKeyPress[blob_ptr->UID]);
#else
            midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
            node_ptr->midiMsg.status = midi::NoteOff;                               // Set MIDI message status to NOTE_OFF
            node_ptr->midiMsg.data1 = lastKeyPress[blob_ptr->UID];                  // Set the note
            node_ptr->midiMsg.data2 = 0;                                            // Set the velocity
            llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
            lastKeyPress[blob_ptr->UID] = -1;                                       // RAZ last key pressed value
          };

#if DEBUG_MAPPING
          Serial.printf("\nGRID\tBLOB:%d\t\tKEY_PRESS:\t%d", blob_ptr->UID, gridLayout[keyPress]);
#else
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);    // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::NoteOn;                                  // Set MIDI message status to NOTE_ON
          node_ptr->midiMsg.data1 = gridLayout[keyPress];                           // Set the note
          node_ptr->midiMsg.data2 = 127;                                            // Set the velocity
          llist_push_front(&midiOut, node_ptr);                                     // Add the node to the midiOut linked liste
#endif
          lastKeyPress[blob_ptr->UID] = gridLayout[keyPress];                       // Keep track of last key pressed to switch it OFF when sliding
          //}; // (Test if the blob is within the key limits)
        };
      }
      else { // if !blob_ptr->state
#if DEBUG_MAPPING
        Serial.printf("\nGRID\tBLOB:%d\tKEY_UP:%d", blob_ptr->UID, lastKeyPress[blob_ptr->UID]);
#else
        midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);      // Get a node from the MIDI nodes stack
        node_ptr->midiMsg.status = midi::NoteOff;                                   // Set MIDI message status to NOTE_OFF
        node_ptr->midiMsg.data1 = lastKeyPress[blob_ptr->UID];                      // Set the note
        node_ptr->midiMsg.data2 = 0;                                                // Set the velocity to 0
        llist_push_front(&midiOut, node_ptr);                                       // Add the node to the midiOut linked liste
#endif
        lastKeyPress[blob_ptr->UID] = -1;                                           // RAZ last key pressed value
      };
    };
  };
};

// Populate the MIDI grid layaout with the incomming MIDI notes
void mapping_grid_populate(void) {
  boolean newNote = false;
  while (1) {
    midiNode_t* nodeIn_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiIn);
    if (nodeIn_ptr != NULL) {
      switch (nodeIn_ptr->midiMsg.status) {
        case midi::NoteOn:
          // Move the input MIDI node to the midiChord linked list
          llist_push_front(&midiChord, llist_pop_front(&midiIn));
          newNote = true;
          break;
        case midi::NoteOff:
          // Remove and save the MIDI node from the midiChord linked list
          // Save the incoming NOTE_OFF MIDI node from the midiIn linked list
          midiNode_t* prevNode_ptr = NULL;
          for (midiNode_t* nodeOut_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiChord); nodeOut_ptr != NULL; nodeOut_ptr = (midiNode_t*)ITERATOR_NEXT(nodeOut_ptr)) {
            if (nodeIn_ptr->midiMsg.data1 == nodeOut_ptr->midiMsg.data1) {
              llist_push_front(&midi_node_stack, llist_pop_front(&midiIn));
              llist_extract_node(&midiChord, prevNode_ptr, nodeOut_ptr);
              llist_push_front(&midi_node_stack, nodeOut_ptr);
              break;
            };
            prevNode_ptr = nodeOut_ptr;
          };
          break;
        default:
          break;
      };
    }
    else {
      break;
    };
  };
  if (newNote) {
    newNote = false;
    unsigned int key = 0;
    while (key < GRID_KEYS) {
      for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiChord); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
        gridLayout[key] = node_ptr->midiMsg.data1;
        key++;
      };
    };
  };
};

#define VSLIDERS 2
rect_t vSlider[VSLIDERS] = {0};
vSlider_t vSliderParams[VSLIDERS] = {
  {10, 10, 40, 10, 44, 0},  // ARGS[posX, Ymin, Ymax, width, cChnage, lastVal]
  {30, 10, 40, 10, 33, 0}   // ARGS[posX, Ymin, Ymax, width, cChnage, lastVal]
};

void VSLIDER_SETUP(void) {
  for (uint8_t index = 0; index < VSLIDERS; index++) {
    vSlider[index].Xmin = vSliderParams[index].posX - round(vSliderParams[index].width / 2);
    vSlider[index].Xmax = vSliderParams[index].posX + round(vSliderParams[index].width / 2);
    vSlider[index].Ymin = vSliderParams[index].Ymin;
    vSlider[index].Ymax = vSliderParams[index].Ymax;
  };
};

void mapping_vSliders(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t index = 0; index < VSLIDERS; index++) {
      if (blob_ptr->centroid.X > vSlider[index].Xmin &&
          blob_ptr->centroid.X < vSlider[index].Xmax &&
          blob_ptr->centroid.Y > vSlider[index].Ymin &&
          blob_ptr->centroid.Y < vSlider[index].Ymax) {
        uint8_t val = round(map(blob_ptr->centroid.Y, vSlider[index].Ymin, vSlider[index].Ymax, 0, 127)); // [0:127]
        if (val != vSliderParams[index].lastVal) {
          vSliderParams[index].lastVal = val;
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);    // Get a node from the MIDI nodes stack
          node_ptr->midiMsg.status = midi::ControlChange;                           // Set MIDI message status to MIDI_CONTROL_CHANGE
          node_ptr->midiMsg.data1 = vSliderParams[index].cChange;                   // Set the note
          node_ptr->midiMsg.data2 = val;                                            // Set the velocity to 0
          node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                          // Set the channel see config.h
          llist_push_front(&midiOut, node_ptr);                                     // Add the node to the midiOut linked liste
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
  {15, 15, 40, 10, 16, 0},  // ARGS[posY, Xmin, Xmax, height, cChange, lastVal]
  {30, 15, 40, 10, 45, 0}   // ARGS[posY, Xmin, Xmax, height, cChange, lastVal]
};

void HSLIDER_SETUP(void) {
  for (uint8_t index = 0; index < VSLIDERS; index++) {
    hSlider[index].Xmin = hSliderParams[index].Xmin;
    hSlider[index].Xmax = hSliderParams[index].Xmax;
    hSlider[index].Ymin = hSliderParams[index].posY - hSliderParams[index].height / 2;
    hSlider[index].Ymax = hSliderParams[index].posY + hSliderParams[index].height / 2;
  };
};

void mapping_hSliders(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t index = 0; index < HSLIDERS; index++) {
      if (blob_ptr->centroid.X > hSlider[index].Xmin &&
          blob_ptr->centroid.X < hSlider[index].Xmax &&
          blob_ptr->centroid.Y > hSlider[index].Ymin &&
          blob_ptr->centroid.Y < hSlider[index].Ymax) {
        uint8_t val = round(map(blob_ptr->centroid.X, hSlider[index].Xmin, hSlider[index].Xmax, 0, 127)); // [0:127]
        if (val != hSliderParams[index].lastVal) {
          hSliderParams[index].lastVal = val;
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);   // Get a node from the MIDI nodes stack
          node_ptr->midiMsg.status = midi::ControlChange;                          // Set MIDI message status to MIDI_CONTROL_CHANGE
          node_ptr->midiMsg.data1 = vSliderParams[index].cChange;                  // Set the note
          node_ptr->midiMsg.data2 = val;                                           // Set the velocity to 0
          node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                         // Set the channel see config.h
          llist_push_front(&midiOut, node_ptr);                                    // Add the node to the midiOut linked liste
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
  {1, 0, 0  },      // PARAMS[sliders, index, track-offset-rad]
  {1, 1, 1  },      // PARAMS[sliders, index, track-offset-rad]
  {2, 2, 2.4},      // PARAMS[sliders, index, track-offset-rad]
  {3, 4, 3.0}       // PARAMS[sliders, index, track-offset-rad]
};

cSlider_t cSliders[CS_SLIDERS] = {
  {0,    IIPi, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {0,    IIPi, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {0,    3.60, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {3.80, IIPi, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {0,    3.20, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {3.40, 5.20, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {5.30, 8, IIPi}   // PARAMS[thetaMin, thetaMax, lastVal]
};

cSlider_t* csMapping[CS_SLIDERS] = {NULL};

void CSLIDER_SETUP(void) {
  //Serial.printf("\nDEBUG_CSLIDER_SETUP\tCS_RADIUS:\t%f", CS_RADIUS);
  //Serial.printf("\nDEBUG_CSLIDER_SETUP\tCS_RMAX:\t%f", CS_RMAX);
  //Serial.printf("\nDEBUG_CSLIDER_SETUP\tCS_WIDTH:\t%f", CS_TRACK_WIDTH);
  //Serial.printf("\nDEBUG_CSLIDER_SETUP\tCS_SCALE_FACTOR:\t%f", CS_SCALE_FACTOR);
};

void mapping_cSlider(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
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
              uint8_t sliderVal = (uint8_t)map(theta, cSliders[id].thetaMin, cSliders[id].thetaMax, 0, 127);
#if DEBUG_MAPPING
              Serial.printf("\nDEBUG_CSLIDER:\tBlob:\t%d\tSlider:\t%d", blob_ptr->UID, id);
#else
              midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
              node_ptr->midiMsg.status = midi::ControlChange;                         // Set MIDI message status to MIDI_CONTROL_CHANGE
              node_ptr->midiMsg.data1 = id;                                           // Set the note
              node_ptr->midiMsg.data2 = sliderVal;                                    // Set the velocity to 0
              node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                        // Set the channel see config.h
              llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
            };
          };
        }
        else {
          cSlider_t* cSlider_ptr = csMapping[blob_ptr->UID];
          if (cSlider_ptr != NULL) {
            if (theta > cSlider_ptr->thetaMin && theta < cSlider_ptr->thetaMax) {
              uint8_t sliderVal = (uint8_t)map(theta, cSlider_ptr->thetaMin, cSlider_ptr->thetaMax, 0, 127);
              if (sliderVal != cSlider_ptr->lastVal) {
#if DEBUG_MAPPING
                Serial.printf("\nDEBUG_CSLIDER:\tRadius:\t%f\tTheta:\t%f\tVal:\t%d", radius, theta, cSlider_ptr->lastVal);
#else
                midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);                               // Get a node from the MIDI nodes stack
                node_ptr->midiMsg.status = midi::ControlChange;                                                      // Set MIDI message status to MIDI_CONTROL_CHANGE
                //node_ptr->midiMsg.data1 = id;                                                                      // Set the note
                node_ptr->midiMsg.data2 = (uint8_t)map(theta, cSlider_ptr->thetaMin, cSlider_ptr->thetaMax, 0, 127); // Set the velocity to 0
                node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                                                     // Set the channel see config.h
                llist_push_front(&midiOut, node_ptr);                                                                // Add the node to the midiOut linked liste
#endif
              };
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
  {0, BZ, 42, 0}, // DETUNE // ARGS[blobID, mappVal[BX,BY,BW,BH,BD], cChange, lastVal]
  {1, BZ, 44, 0}  // CUTOFF // ARGS[blobID, mappVal[BX,BY,BW,BH,BD], cChange, lastVal]
};

void mapping_blob(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (int index = 0; index < CCHANGE; index++) {
      if (blob_ptr->UID == cChange[index].blobID) {
        // Test if the blob is alive
        if (blob_ptr->state) {
          switch (cChange[index].mappVal) {
            case BX:
              if (blob_ptr->centroid.X != cChange[index].lastX) {
#if DEBUG_MAPPING
                Serial.printf("\nMIDI\tCC_BX:%d", map(blob_ptr->centroid.X, X_MIN, X_MAX, 0, 127));
#else
                midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);       // Get a node from the MIDI node stack
                node_ptr->midiMsg.status = midi::ControlChange;                              // Set MIDI message status to CONTROL_CHANGE
                node_ptr->midiMsg.data1 = cChange[index].cChange;                            // Set the note
                node_ptr->midiMsg.data2 = map(blob_ptr->centroid.X, X_MIN, X_MAX, 0, 127);   // Set the value
                llist_push_front(&midiOut, node_ptr);                                        // Add the node to the midiOut linked liste
#endif
                cChange[index].lastX = blob_ptr->centroid.X;
              };
              break;
            case BY:
              if (blob_ptr->centroid.Y != cChange[index].lastY) {
#if DEBUG_MAPPING
                Serial.printf("\nMIDI\tCC_BY:%d", map(blob_ptr->centroid.Y, Y_MIN, Y_MAX, 0, 127));
#else
                midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);       // Get a node from the MIDI node stack
                node_ptr->midiMsg.status = midi::ControlChange;                              // Set MIDI message status to CONTROL_CHANGE
                node_ptr->midiMsg.data1 = cChange[index].cChange;                            // Set the note
                node_ptr->midiMsg.data2 = map(blob_ptr->centroid.Y, Y_MIN, Y_MAX, 0, 127);   // Set the value
                llist_push_front(&midiOut, node_ptr);                                        // Add the node to the midiOut linked liste
#endif
                cChange[index].lastY = blob_ptr->centroid.Y;
              };
              break;
            case BZ:
              if (blob_ptr->centroid.Z != cChange[index].lastZ) {
#if DEBUG_MAPPING
                Serial.printf("\nMIDI\tCC_BZ:%d", constrain(blob_ptr->centroid.Z, 0, 127));
#else
                midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);       // Get a node from the MIDI node stack
                node_ptr->midiMsg.status = midi::ControlChange;                              // Set MIDI message status to CONTROL_CHANGE
                node_ptr->midiMsg.data1 = cChange[index].cChange;                            // Set the note
                node_ptr->midiMsg.data2 = constrain(blob_ptr->centroid.Z, 0, 127);           // Set the value
                llist_push_front(&midiOut, node_ptr);                                        // Add the node to the midiOut linked liste
#endif
                cChange[index].lastZ = constrain(blob_ptr->centroid.Z, 0, 127);
              };
              break;
            case BW:
              if (blob_ptr->box.W != cChange[index].lastW) {
#if DEBUG_MAPPING
                Serial.printf("\nMIDI\tCC_BW:%d", blob_ptr->box.W);
#else
                midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);       // Get a node from the MIDI node stack
                node_ptr->midiMsg.status = midi::ControlChange;                              // Set MIDI message status to CONTROL_CHANGE
                node_ptr->midiMsg.data1 = cChange[index].cChange;                            // Set the note
                node_ptr->midiMsg.data2 = blob_ptr->box.W;                                   // Set the value
                llist_push_front(&midiOut, node_ptr);                                        // Add the node to the midiOut linked liste
#endif
                cChange[index].lastW = blob_ptr->box.W;
              };
              break;
            case BH:
              if (blob_ptr->box.H != cChange[index].lastH) {
#if DEBUG_MAPPING
                Serial.printf("\nMIDI\tCC_BH:%d", blob_ptr->box.H);
#else
                midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);       // Get a node from the MIDI node stack
                node_ptr->midiMsg.status = midi::ControlChange;                              // Set MIDI message status to CONTROL_CHANGE
                node_ptr->midiMsg.data1 = cChange[index].cChange;                            // Set the note
                node_ptr->midiMsg.data2 = blob_ptr->box.H;                                   // Set the value
                llist_push_front(&midiOut, node_ptr);                                        // Add the node to the midiOut linked liste
#endif
                cChange[index].lastH = blob_ptr->box.H;
              };
              break;
            default:
              break;
          };
        };
      };
    };
  };
};
