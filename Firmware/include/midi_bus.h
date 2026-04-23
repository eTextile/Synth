/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MIDI_BUS_H__
#define __MIDI_BUS_H__

#include <MIDI.h>  // https://github.com/FortySevenEffects/arduino_midi_library > https://github.com/PaulStoffregen/MIDI
using namespace midi;

#include "config.h"
#include "llist.h"
#include "blob.h"

extern llist_t llist_midi_nodes_pool; // Main MIDI node stack
extern llist_t llist_midi_in;         // Main MIDI Input linked list
extern llist_t llist_midi_out;        // Main MIDI Output linked list

extern llist_t llist_active_midi_msg; // Main MIDI notes/chords linked list

typedef struct midi_msg_s midi_msg_t;
struct midi_msg_s {
  uint8_t channel;  // [1-15] MIDI channel (Extracted from status byte)
  MidiType type;    // (Extracted from status byte)
  uint8_t data1;    // [0-127] MIDI controller number or note number
  uint8_t data2;    // [0-127] MIDI controller value or velocity
};

typedef struct midi_limit_s midi_limit_t;
struct midi_limit_s {
  uint8_t min; // [0-127]
  uint8_t max; // [0-127]
};

// MIDI status bytes: https://github.com/PaulStoffregen/MIDI/blob/master/src/midi_Defs.h
typedef struct midi_status_s midi_status_t;
struct midi_status_s {
  MidiType type;
  uint8_t channel;
};

void midi_bus_setup(void);

uint8_t midi_msg_status_pack(MidiType type, uint8_t channel);
void midi_msg_status_unpack(uint8_t in_status, midi_status_t* out_status);

const char* get_type_name(MidiType code);

void print_bytes(const uint8_t* data_ptr, size_t length);

#endif /*__MIDI_BUS_H__*/
