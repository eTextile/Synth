/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping_lib.h"

void MAPPING_LIB_SETUP(){
  MAPPING_TOUCHPADS_SETUP();
  MAPPING_POLYGONS_SETUP();
  MAPPING_CIRCLES_SETUP();
  MAPPING_TRIGGERS_SETUP();
  MAPPING_TOGGLES_SETUP();
  MAPPING_VSLIDERS_SETUP();
  MAPPING_HSLIDERS_SETUP();
  //MAPPING_GRID_SETUP();
  //MAPPING_CSLIDERS_SETUP();
};

///////////////////////////////////////////////
              /*__TOGGLES__*/
///////////////////////////////////////////////
uint8_t mapp_togs = 0;
Key_t *mapp_togsParams = NULL;
static Key_t mapp_togsParams_privStore[MAX_TOGGLES];

void mapping_toggles_alloc(uint8_t count) {
  mapp_togs = min(count, MAX_TOGGLES);
  mapp_togsParams = mapp_togsParams_privStore;
};

void MAPPING_TOGGLES_SETUP(void) {
  // Nothing to pre-compute yet
};

void mapping_toggles_update(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t i = 0; i < mapp_togs; i++) {
      // Test if the blob is within the key limits
      if (blob_ptr->centroid.X > mapp_togsParams[i].rect.Xmin &&
          blob_ptr->centroid.X < mapp_togsParams[i].rect.Xmax &&
          blob_ptr->centroid.Y > mapp_togsParams[i].rect.Ymin &&
          blob_ptr->centroid.Y < mapp_togsParams[i].rect.Ymax) {
        if (!blob_ptr->lastState) {
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TOGGLES\tID:%d\tNOTE_ON:%d", i, toggleParam[i].note);
#else
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::NoteOn;                                // Set MIDI message status to NOTE_OFF
          node_ptr->midiMsg.data1 = mapp_togsParams[i].note;                      // Set the note
          //node_ptr->midiMsg.data2 = blob_ptr->velocity.Z                        // Set the velocity
          node_ptr->midiMsg.data2 = 127;                                          // Set the velocity
          node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                        // Set the channel see config.h
          llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
        };
        if (!blob_ptr->state) {
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TOGGLES\tID:%d\tNOTE_OFF:%d", i, toggleParam[i].note);
#else
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::NoteOff;                               // Set MIDI message status to NOTE_OFF
          node_ptr->midiMsg.data1 = mapp_togsParams[i].note;                     // Set the note
          node_ptr->midiMsg.data2 = 0;                                            // Set the velocity
          node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                        // Set the channel see config.h
          llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
        };
      };
    };
  };
};

///////////////////////////////////////////////
              /*__TRIGGERS__*/
///////////////////////////////////////////////
uint8_t mapp_trigs = 0;
Key_t *mapp_trigsParams = NULL;
static Key_t mapp_trigsParams_privStore[MAX_TRIGGERS];

void mapping_triggers_alloc(uint8_t count) {
  mapp_trigs = min(count, MAX_TRIGGERS);
  mapp_trigsParams = mapp_trigsParams_privStore;
};

inline void MAPPING_TRIGGERS_SETUP(void) {
  // Nothing to pre-compute yet
};

void mapping_triggers_update(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t i = 0; i < mapp_trigs; i++) {
      // Test if the blob is within the key limits
      if (blob_ptr->centroid.X > mapp_trigsParams[i].rect.Xmin &&
          blob_ptr->centroid.X < mapp_trigsParams[i].rect.Xmax &&
          blob_ptr->centroid.Y > mapp_trigsParams[i].rect.Ymin &&
          blob_ptr->centroid.Y < mapp_trigsParams[i].rect.Ymax) {
        if (!blob_ptr->lastState) {
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::NoteOn;                                // Set MIDI message status to NOTE_OFF
          node_ptr->midiMsg.data1 = mapp_trigsParams[i].note;                     // Set the note
          //node_ptr->midiMsg.data2 = blob_ptr->velocity.Z                        // TODO: Set the velocity
          node_ptr->midiMsg.data2 = 127;                                          // Set the velocity
          node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                        // Set the channel see config.h
          llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TRIGGERS\tID:%d\tNOTE_ON:%d", i, mapp_trigParams[i].note);
#endif
        }
        else if (!blob_ptr->state) {
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::NoteOff;                               // Set MIDI message status to NOTE_OFF
          node_ptr->midiMsg.data1 = mapp_trigsParams[i].note;                     // Set the note
          node_ptr->midiMsg.data2 = 0;                                            // Set the velocity
          node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                        // Set the channel see config.h
          llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TRIGGERS\tID;%d\tNOTE_OFF:%d", i, mapp_trigParams[i].note);
#endif
        };
      };
    };
  };
};

///////////////////////////////////////////////
              /*__V_SLIDERS__*/
///////////////////////////////////////////////
uint8_t mapp_vSliders = 0;
vSlider_t* mapp_vSlidersParams = NULL;
static vSlider_t mapp_vSlidersParams_privStore[MAX_VSLIDERS];

uint8_t mapp_vSliders_lastVal[MAX_VSLIDERS];

void mapping_vSliders_alloc(uint8_t count) {
  mapp_vSliders = min(count, MAX_VSLIDERS);
  mapp_vSlidersParams = mapp_vSlidersParams_privStore;
};

void MAPPING_VSLIDERS_SETUP(void) {
  // Nothing to pre-compute yet
};

void mapping_vSliders_update(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t i = 0; i < mapp_vSliders; i++) {
      if (blob_ptr->centroid.X > mapp_vSlidersParams[i].rect.Xmin &&
          blob_ptr->centroid.X < mapp_vSlidersParams[i].rect.Xmax &&
          blob_ptr->centroid.Y > mapp_vSlidersParams[i].rect.Ymin &&
          blob_ptr->centroid.Y < mapp_vSlidersParams[i].rect.Ymax) {
        uint8_t val = round(map(blob_ptr->centroid.Y, mapp_vSlidersParams[i].rect.Ymin, mapp_vSlidersParams[i].rect.Ymax, 0, 127)); // [0:127]
        if (val != mapp_vSliders_lastVal[i]) {
          mapp_vSliders_lastVal[i] = val;
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_VSLIDER\tID:%d\tVal:%d", i, val);
#else
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);    // Get a node from the MIDI nodes stack
          node_ptr->midiMsg.status = midi::ControlChange;                           // Set MIDI message status to MIDI_CONTROL_CHANGE
          node_ptr->midiMsg.data1 = mapp_vSlidersParams[i].CC;                 // Set the note
          node_ptr->midiMsg.data2 = val;                                            // Set the velocity to 0
          node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                          // Set the channel see config.h
          llist_push_front(&midiOut, node_ptr);                                     // Add the node to the midiOut linked liste
#endif
        };
        break;
      };
    };
  };
};

///////////////////////////////////////////////
              /*__H_SLIDERSS__*/
///////////////////////////////////////////////
uint8_t mapp_hSliders = 0;
hSlider_t* mapp_hSlidersParams = NULL;
static hSlider_t mapp_hSlidersParams_privStore[MAX_HSLIDERS];

uint8_t mapp_hSliders_lastVal[MAX_HSLIDERS];

void mapping_hSliders_alloc(uint8_t count) {
  mapp_hSliders = min(count, MAX_HSLIDERS);
  mapp_hSlidersParams = mapp_hSlidersParams_privStore;
};

void MAPPING_HSLIDERS_SETUP(void) {
  // Nothing to pre-compute yet
};

void mapping_hSliders_update(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t index = 0; index < mapp_hSliders; index++) {
      if (blob_ptr->centroid.X > mapp_hSlidersParams[index].rect.Xmin &&
          blob_ptr->centroid.X < mapp_hSlidersParams[index].rect.Xmax &&
          blob_ptr->centroid.Y > mapp_hSlidersParams[index].rect.Ymin &&
          blob_ptr->centroid.Y < mapp_hSlidersParams[index].rect.Ymax) {
        uint8_t val = round(map(blob_ptr->centroid.X, mapp_hSlidersParams[index].rect.Xmin, mapp_hSlidersParams[index].rect.Xmax, 0, 127)); // [0:127]
        if (val != mapp_hSliders_lastVal[index]) {
          mapp_hSliders_lastVal[index] = val;
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_HSLIDER\tID:%d\tVal:%d", i, val);
#else
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);   // Get a node from the MIDI nodes stack
          node_ptr->midiMsg.status = midi::ControlChange;                          // Set MIDI message status to MIDI_CONTROL_CHANGE
          node_ptr->midiMsg.data1 = mapp_hSlidersParams[index].CC;            // Set the note
          node_ptr->midiMsg.data2 = val;                                           // Set the velocity to 0
          node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;                         // Set the channel see config.h
          llist_push_front(&midiOut, node_ptr);                                    // Add the node to the midiOut linked liste
#endif
        };
        break;
      };
    };
  };
};

///////////////////////////////////////////////
              /*__CIRCLES__*/
///////////////////////////////////////////////
uint8_t mapp_circles = 0;
circle_t *mapp_circlesParams = NULL;
static circle_t mapp_circlesParams_privStore[MAX_CIRCLES];

void mapping_circles_alloc(uint8_t count) {
  mapp_circles = min(count, MAX_CIRCLES);
  mapp_circlesParams = mapp_circlesParams_privStore;
};

void MAPPING_CIRCLES_SETUP(void){
   // Nothing to pre-compute yet
};

void mapping_circles_update(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (int i = 0; i < mapp_circles; i++) {
      float x = blob_ptr->centroid.X - mapp_circlesParams[i].radius;
      float y = blob_ptr->centroid.Y - mapp_circlesParams[i].radius;
      float radius = sqrt(x * x + y * y);
      float theta = 0;
      if (radius < mapp_circlesParams[i].radius) {
        // Rotation of Axes through an angle without shifting Origin
        float posX = x * cos(mapp_circlesParams[i].offset) + y * sin(mapp_circlesParams[i].offset);
        float posY = -x * sin(mapp_circlesParams[i].offset) + y * cos(mapp_circlesParams[i].offset);
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
#if defined(DEBUG_MAPPING)
        Serial.printf("\nDEBUG_MAPPING_CIRCLES:\tCircleID:\t%d\tradius:\t%fTheta:\t%f", i, radius, theta);
#else
        midiNode_t* node_ptr = NULL;
          node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);    // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::NoteOff;                     // Set MIDI message status to NOTE_OFF
          //node_ptr->midiMsg.data1 = mapp_circlesParams[i].CCradius ;  // Set the control change
          node_ptr->midiMsg.data2 = theta;                              // Set the velocity
          node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;              // Set the channel see config.h
          llist_push_front(&midiOut, node_ptr);                         // Add the node to the midiOut linked liste
          node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);    // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::NoteOff;                     // Set MIDI message status to NOTE_OFF
          //node_ptr->midiMsg.data1 = mapp_circlesParams[i].CCtheta;    // Set the control change
          node_ptr->midiMsg.data2 = radius;                             // Set the velocity
          node_ptr->midiMsg.channel = MIDI_OUTPUT_CHANNEL;              // Set the channel see config.h
          llist_push_front(&midiOut, node_ptr);                         // Add the node to the midiOut linked liste
#endif
      };
    };
  };
};

///////////////////////////////////////////////
              /*__TOUCHPAD__*/
///////////////////////////////////////////////
uint8_t mapp_touchpads = 0;
touchpad_t* mapp_touchpadsParams = NULL;
static touchpad_t mapp_touchpadsParams_privStore[MAX_POLYGONS];

void mapping_touchpads_alloc(uint8_t count) {
  mapp_touchpads = min(mapp_touchpads, MAX_POLYGONS);
  mapp_touchpadsParams = mapp_touchpadsParams_privStore;
};

void MAPPING_TOUCHPADS_SETUP(void){
  // Nothing to pre-compute yet
};

void mapping_touchpads_update(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (uint8_t i = 0; i < mapp_touchpads; i++) {
      if (blob_ptr->centroid.X > mapp_touchpadsParams[i].rect.Xmin &&
          blob_ptr->centroid.X < mapp_touchpadsParams[i].rect.Xmax &&
          blob_ptr->centroid.Y > mapp_touchpadsParams[i].rect.Ymin &&
          blob_ptr->centroid.Y < mapp_touchpadsParams[i].rect.Ymax) {
        if (mapp_touchpadsParams[i].CCx) {
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TOUCHPAD\tMIDI_CCx:%d\tVAL:%d", mapp_touchpadsParams[i].CCx, round(map(blob_ptr->centroid.X, mapp_touchpadsParams[i].rect.Xmin, mapp_touchpadsParams[i].rect.Xmax, 0, 127)));
#else
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::ControlChange;                         // Set MIDI message status to CONTROL_CHANGE
          node_ptr->midiMsg.data1 = mapp_touchpadsParams[i].CCx;                  // Set the note
          node_ptr->midiMsg.data2 = round(map(blob_ptr->centroid.X, mapp_touchpadsParams[i].rect.Xmin, mapp_touchpadsParams[i].rect.Xmax, 0, 127));
          llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
        };
        if (mapp_touchpadsParams[i].CCy) {
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TOUCHPAD\tMIDI_CCy:%d\tVAL:%d", mapp_touchpadsParams[i].CCy, round(map(blob_ptr->centroid.Y, mapp_touchpadsParams[i].rect.Ymin, mapp_touchpadsParams[i].rect.Ymax, 0, 127)));
#else
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::ControlChange;                         // Set MIDI message status to CONTROL_CHANGE
          node_ptr->midiMsg.data1 = mapp_touchpadsParams[i].CCy;                  // Set the note
          node_ptr->midiMsg.data2 = round(map(blob_ptr->centroid.Y, mapp_touchpadsParams[i].rect.Ymin, mapp_touchpadsParams[i].rect.Ymax, 0, 127));
          llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
        };
        if (mapp_touchpadsParams[i].CCz) {
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TOUCHPAD\tMIDI_CCz:%d\tVAL:%d", mapp_touchpadsParams[i].CCz, map(blob_ptr->centroid.Z, 0, 255, 0, 127));
#else
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::ControlChange;                         // Set MIDI message status to CONTROL_CHANGE
          node_ptr->midiMsg.data1 = mapp_touchpadsParams[i].CCz;                  // Set the note
          node_ptr->midiMsg.data2 = map(blob_ptr->centroid.Z, 0, 255, 0, 127);
          llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
        };
        if (mapp_touchpadsParams[i].CCs) {
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_TOUCHPAD\tMIDI_CCs:%d\tVAL:%d", mapp_touchpadsParams[i].CCs, map(blob_ptr->box.W * blob_ptr->box.H, BLOB_MIN_PIX, BLOB_MAX_PIX, 0, 127));
#else
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::ControlChange;                         // Set MIDI message status to CONTROL_CHANGE
          node_ptr->midiMsg.data1 = mapp_touchpadsParams[i].CCs;                  // Set the note
          node_ptr->midiMsg.data2 = map(blob_ptr->box.W * blob_ptr->box.H, BLOB_MIN_PIX, BLOB_MAX_PIX, 0, 127);
          llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
        };
        //...
      };
    };
  };
};

///////////////////////////////////////////////
              /*__POLYGONS__*/ 
///////////////////////////////////////////////
// Algorithm: http://alienryderflex.com/polygon
uint8_t mapp_polygons = 0;
polygon_t* mapp_polygonsParams = NULL;
static polygon_t mapp_polygonsParams_privStore[MAX_POLYGONS];

void mapping_polygons_alloc(uint8_t count) {
  mapp_polygons = min(mapp_polygons, MAX_POLYGONS);
  mapp_polygonsParams = mapp_polygonsParams_privStore;
};

// For line equation y = mx + c, we pre-compute m and c for all edges of a given polygon
void MAPPING_POLYGONS_SETUP(void) {
  uint8_t p = 0;
  uint8_t v1, v2 = (mapp_polygonsParams[p].vertices_cnt - 1);
  float x1, x2, y1, y2;
  for (p = 0; p < mapp_polygons; p++) {
    for (v1 = 0; v1 < mapp_polygonsParams[p].vertices_cnt; v1++) {
      x1 = mapp_polygonsParams[p].vertices[v1].x;
      y1 = mapp_polygonsParams[p].vertices[v1].y;
      x2 = mapp_polygonsParams[p].vertices[v2].x;
      y2 = mapp_polygonsParams[p].vertices[v2].y;

      if (y2 == y1) {
        mapp_polygonsParams[p].c[v1] = x1;
        mapp_polygonsParams[p].m[v1] = 0;
      }
      else {
        mapp_polygonsParams[p].c[v1] = x1 - (y1 * x2) / (y2 - y1) + (y1 * x1) / (y2 - y1);
        mapp_polygonsParams[p].m[v1] = (x2 - x1) / (y2 - y1);
      };
      v2 = v1;
    };
  };
};

void mapping_polygons_update(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    bool odd_nodes = false;
    uint8_t p = 0;
    int i, j = (mapp_polygonsParams[p].vertices_cnt - 1);

    for (p = 0; p < mapp_polygons; p++) {
      odd_nodes = false;
      for (i = 0; i < mapp_polygonsParams[p].vertices_cnt; i++) {
        //float X1 = mapp_polygonsParams[p].vertices[i].x;
        float Y1 = mapp_polygonsParams[p].vertices[i].y;
        //float X2 = mapp_polygonsParams[p].vertices[j].x;
        float Y2 = mapp_polygonsParams[p].vertices[j].y;
        if ((Y1 < blob_ptr->centroid.Y && Y2 >= blob_ptr->centroid.Y) || (Y2 < blob_ptr->centroid.Y && Y1 >= blob_ptr->centroid.Y)) {
          odd_nodes ^= ((blob_ptr->centroid.Y * mapp_polygonsParams[p].m[i] + mapp_polygonsParams[p].c[i]) < blob_ptr->centroid.X);
        };
        j = i;
      };
      if (odd_nodes) {
          // TODO: get the max width & max height and scale it to [0-1]
#if defined(DEBUG_MAPPING)
    Serial.printf("\nDEBUG_MAPPING_POLYGONS:\tPoint %f %f is inside polygon %d\n", blob_ptr->centroid.X, blob_ptr->centroid.Y, p);
#else

#endif
        break;
      };
    };
    if (p == mapp_polygons) {
#if defined(DEBUG_MAPPING)
      printf("\nDEBUG_MAPPING_POLYGONS:\tPoint %f %f does not lie within any polygon\n", blob_ptr->centroid.X, blob_ptr->centroid.Y);
#endif
    };
  };
};

///////////////////////////////////////////////
              /*__GRID__*/ 
///////////////////////////////////////////////
#define GRID_GAP               (float)0.4
#define KEY_SIZE_X             (float)((X_MAX - (GRID_GAP * (GRID_COLS + 1))) / GRID_COLS)
#define KEY_SIZE_Y             (float)((Y_MAX - (GRID_GAP * (GRID_ROWS + 1))) / GRID_ROWS)
#define GRID_X_SCALE_FACTOR    ((float)1/X_MAX) * GRID_COLS
#define GRID_Y_SCALE_FACTOR    ((float)1/Y_MAX) * GRID_ROWS

Key_t mapping_grid_keys[GRID_KEYS] = {0};     // ARGS[posX, Ymin, Ymax, width, state] : 1D array to store keys limits & state
int16_t lastKeyPress[MAX_BLOBS] = {0};        // 1D array to store last keys pressed value

unsigned long int lastTime[MAX_BLOBS] = {0};  // 1D array to store last keys pressed value

// Pre-compute key min & max coordinates
void MAPPING_GRID_SETUP(void) {
  for (uint8_t row = 0; row < GRID_ROWS; row++) {
    uint8_t rowPos = row * GRID_COLS;
    for (uint8_t col = 0; col < GRID_COLS; col++) {
      uint8_t keyPos = rowPos + col;
      mapping_grid_keys[keyPos].rect.Xmin = col * KEY_SIZE_X + (col + 1) * GRID_GAP;
      mapping_grid_keys[keyPos].rect.Xmax = mapping_grid_keys[keyPos].rect.Xmin + KEY_SIZE_X;
      mapping_grid_keys[keyPos].rect.Ymin = row * KEY_SIZE_Y + (row + 1) * GRID_GAP;
      mapping_grid_keys[keyPos].rect.Ymax = mapping_grid_keys[keyPos].rect.Ymin + KEY_SIZE_Y;
#if defined(DEBUG_MAPPING)
      Serial.printf("\nDEBUG_MAPPING_GRID\tkey:%d\t_Xmin:%f\t_Xmax:%f\t_Ymin:%f\t_Ymax:%f",
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
#if defined(DEBUG_MAPPING)
          Serial.printf("\nDEBUG_MAPPING_GRID\tBLOB_ID:%d\tKEY_SLIDING_OFF:%d", blob_ptr->UID, lastKeyPress[blob_ptr->UID]);
#else
          midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI node stack
          node_ptr->midiMsg.status = midi::NoteOff;                               // Set MIDI message status to NOTE_OFF
          node_ptr->midiMsg.data1 = lastKeyPress[blob_ptr->UID];                  // Set the note
          node_ptr->midiMsg.data2 = 0;                                            // Set the velocity
          llist_push_front(&midiOut, node_ptr);                                   // Add the node to the midiOut linked liste
#endif
          lastKeyPress[blob_ptr->UID] = -1;                                       // RAZ last key pressed value
        };
#if defined(DEBUG_MAPPING)
        Serial.printf("\nDEBUG_MAPPING_GRID\tBLOB_ID:%d\tKEY_PRESS:%d", blob_ptr->UID, gridLayout[keyPress]);
#else
        midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);    // Get a node from the MIDI node stack
        node_ptr->midiMsg.status = midi::NoteOn;                                  // Set MIDI message status to NOTE_ON
        node_ptr->midiMsg.data1 = gridLayout[keyPress];                           // Set the note
        node_ptr->midiMsg.data2 = 127;                                            // Set the velocity
        llist_push_front(&midiOut, node_ptr);                                     // Add the node to the midiOut linked liste
#endif
        lastKeyPress[blob_ptr->UID] = gridLayout[keyPress];                       // Keep track of last key pressed to switch it OFF when sliding
      };
    }
    else { // if !blob_ptr->state
#if defined(DEBUG_MAPPING)
      Serial.printf("\nDEBUG_MAPPING_GRID\tBLOB_ID:%d\tKEY_UP:%d", blob_ptr->UID, lastKeyPress[blob_ptr->UID]);
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
        //default:
        //break;
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

///////////////////////////////////////////////
              /*__C_SLIDERS__*/ 
///////////////////////////////////////////////
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

cSlider_t mapp_cSliders[CS_SLIDERS] = {
  {0,    IIPi, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {0,    IIPi, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {0,    3.60, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {3.80, IIPi, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {0,    3.20, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {3.40, 5.20, 0},  // PARAMS[thetaMin, thetaMax, lastVal]
  {5.30, 8, IIPi}   // PARAMS[thetaMin, thetaMax, lastVal]
};

cSlider_t* mapp_cSliders_ptr[CS_SLIDERS] = {NULL};

void MAPPING_CSLIDERS_SETUP() {
  // Nothing to pre-compute yet
}

void mapping_cSliders_update(void) {
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
#if defined(DEBUG_MAPPING)
      Serial.printf("\nDEBUG_MAPPING_CSLIDER\tRADIUS:%f\tTHETA:%f", radius, theta);
#endif
      if (blob_ptr->state) {
        if (!blob_ptr->lastState) {
          for (uint8_t id = cTrack[track].index; id < cTrack[track].index + cTrack[track].sliders; id++) {
            if (theta > mapp_cSliders[id].thetaMin && theta < mapp_cSliders[id].thetaMax) {
              mapp_cSliders_ptr[blob_ptr->UID] = &mapp_cSliders[id]; // Record pointer to slider
              uint8_t sliderVal = (uint8_t)map(theta, mapp_cSliders[id].thetaMin, mapp_cSliders[id].thetaMax, 0, 127);
#if defined(DEBUG_MAPPING)
              Serial.printf("\nDEBUG_MAPPING_CSLIDER\tSLIDER_ID:%d\tRADIUS:%f\tTHETA:%f\tVAL:%d", id, radius, theta, sliderVal);
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
          cSlider_t* cSlider_ptr = mapp_cSliders_ptr[blob_ptr->UID];
          if (cSlider_ptr != NULL) {
            if (theta > cSlider_ptr->thetaMin && theta < cSlider_ptr->thetaMax) {
              uint8_t sliderVal = (uint8_t)map(theta, cSlider_ptr->thetaMin, cSlider_ptr->thetaMax, 0, 127);
              if (sliderVal != cSlider_ptr->lastVal) {
#if defined(DEBUG_MAPPING)
                Serial.printf("\nDEBUG_MAPPING_CSLIDER\tSLIDER_ID:%d\tRADIUS:%f\tTHETA:%f\tVAL:%d", id, radius, theta, sliderVal);
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
        mapp_cSliders_ptr[blob_ptr->UID] = NULL;
      };
    };
  };
};

void mapping_lib_update(void) {
  llist_save_nodes(&midi_node_stack, &midiOut); // Save/rescure all midiOut nodes
  mapping_touchpads_update();
  mapping_polygons_update();
  mapping_circles_update();
  mapping_triggers_update();
  mapping_toggles_update();
  mapping_vSliders_update();
  mapping_hSliders_update();
  //mapping_grid_populate();
  //mapping_grid_update();
  //mapping_cSliders_update();
};