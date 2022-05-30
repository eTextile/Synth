/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MIDI_BUS_H__
#define __MIDI_BUS_H__

#include "llist.h"

#include <MIDI.h>  // https://github.com/FortySevenEffects/arduino_midi_library > https://github.com/PaulStoffregen/MIDI
using namespace midi;

// Exposed local declarations see midi_transmit.cpp
extern llist_t midi_node_stack;
extern llist_t midiIn;
extern llist_t midiOut;
extern llist_t midiChord;

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
  node_ptr->midiMsg.status = status; // status/comande byte: MSB:status & LSB:channel
  node_ptr->midiMsg.data1 = note;
  node_ptr->midiMsg.data2 = value;
  llist_push_front(&midiOut, node_ptr);
};

void midi_bus_setup(void);

#endif /*__MIDI_BUS_H__*/
