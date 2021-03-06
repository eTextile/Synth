/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/


#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#include <MIDI.h>               // http://www.pjrc.com/teensy/td_midi.html

#include "llist.h"

typedef struct llist llist_t;   // Forward declaration

extern llist_t midi_nodes_stack;
extern llist_t midiIn;
extern llist_t midiOut;

typedef struct midiNode midiNode_t;
struct midiNode {
  lnode_t node;
  uint8_t pithch;
  uint8_t velocity;
  uint8_t channel;
};

void llist_midi_init(llist_t* llist_ptr, midiNode_t* nodesArray_ptr, const int nodes);
void TRANSMIT_SETUP(void);

#endif /*___TRANSMIT_H___*/
