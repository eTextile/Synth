/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "midi_nodes.h"

#define MIDI_NODES 64

midiNode_t midiNodeArray[MIDI_NODES] = {0}; // Memory allocation for all MIDI I/O messages

llist_t midi_node_stack;                    // Main MIDI node stack
llist_t midiIn;                             // Main MIDI Input linked list
llist_t midiOut;                            // Main MIDI Output linked list
llist_t midiChord;                          // Main MIDI chord linked list

void llist_midi_init(llist_t* llist_ptr, midiNode_t* nodeArray_ptr, const int nodes) {
  llist_raz(llist_ptr);
  for (int i = 0; i < nodes; i++) {
    llist_push_front(llist_ptr, &nodeArray_ptr[i]);
  };
};

void MIDI_NODES_SETUP(void) {
    llist_midi_init(&midi_node_stack, &midiNodeArray[0], MIDI_NODES); // Add X nodes to the midi_node_stack
    llist_raz(&midiIn);
    llist_raz(&midiOut);
    llist_raz(&midiChord);
};
