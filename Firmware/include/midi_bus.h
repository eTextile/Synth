/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MIDI_BUS_H__
#define __MIDI_BUS_H__

#include "llist.h"

#include <MIDI.h>  // https://github.com/FortySevenEffects/arduino_midi_library > https://github.com/PaulStoffregen/MIDI
using namespace midi;

extern llist_t midi_node_stack;  // Exposed local declaration see midi_transmit.cpp
extern llist_t midiIn;           // Exposed local declaration see midi_transmit.cpp
extern llist_t midiOut;          // Exposed local declaration see midi_transmit.cpp
extern llist_t midiChord;        // Exposed local declaration see midi_transmit.cpp

typedef struct midiMsg midiMsg_t;
struct midiMsg {
  uint8_t status;   // For MIDI status bytes see: https://github.com/PaulStoffregen/MIDI/blob/master/src/midi_Defs.h
  uint8_t data1;    // First value  (0-127), controller number or note number
  uint8_t data2;    // Second value (0-127), controller value or velocity
  uint8_t channel;  // MIDI channel (0-15)
};

typedef struct midiNode midiNode_t;
struct midiNode {
  lnode_t node;
  midiMsg_t midiMsg;
};

inline void midi_sendOut(uint8_t status, uint8_t note, uint8_t value){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);

  node_ptr->midiMsg.status = status; // Comande byte: MSB:status & LSB:channel
  node_ptr->midiMsg.data1 = note;
  node_ptr->midiMsg.data2 = value;
  llist_push_front(&midiOut, node_ptr);
};

void MIDI_SETUP(void);

#endif /*__MIDI_BUS_H__*/