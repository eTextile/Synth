/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "midi_seq.h"

typedef struct seq_s seq_t;
struct seq_s {
  //uint32_t intervalTime[32];
  uint8_t* seqframe;
};

void tap_tempo(void) {
  /*
  for (lnode_t* midi_node_ptr = ITERATOR_START_FROM_HEAD(&llist_midi_out); midi_node_ptr != NULL; midi_node_ptr = ITERATOR_NEXT(midi_node_ptr)) {
    midi_msg_t* midi_msg_ptr = (midi_msg_t*)ITERATOR_DATA(node_ptr);
    // TODO
  };
  */
};

// TODO
void step_sequencer(void) {
};

// TODO
void arpeggiator(void) {
};
