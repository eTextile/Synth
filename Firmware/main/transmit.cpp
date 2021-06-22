/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "transmit.h"

midiNode_t nodesArray_ptr[MAX_SYNTH] = {0}; // 1D array to mapp incoming midi notes in the grid layout

llist_t midi_nodes_stack;                   // 
llist_t midiIn;                             // Main Midi Input linked list
llist_t midiOut;                            // Main Midi Output linked list

void llist_midi_init(llist_t* llist_ptr, midiNode_t* nodesArray_ptr, const int nodes) {
  llist_raz(llist_ptr);
  for (int i = 0; i < nodes; i++) {
    llist_push_front(llist_ptr, &nodesArray_ptr[i]);
  }
}

void TRANSMIT_SETUP(void) {
  llist_midi_init(&midi_nodes_stack, &nodesArray_ptr[0], MAX_SYNTH); // Add X nodes to the llist_context_stack
  llist_raz(&midiIn);
  llist_raz(&midiOut);
}
