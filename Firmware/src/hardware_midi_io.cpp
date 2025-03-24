/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "hardware_midi_io.h"

#include "config.h"
#include "llist.h"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);

void hardware_midi_setup(void) {
  MIDI.begin(MIDI_INPUT_CHANNEL); // Launch MIDI hardware and listen to channel 1
  MIDI.setHandleMessage(hardware_midi_handle_input);
};

void hardware_midi_recive(void) {
  MIDI.read(MIDI_INPUT_CHANNEL);         // Is there any incoming MIDI messages on channel 1
  while (MIDI.read(MIDI_INPUT_CHANNEL)); // Read and discard any incoming MIDI messages
};

void hardware_midi_handle_input(const Message<128u> &midiMsg) {
  // midiMsg struct is C++
  // Can it be refact for zero-copy ?
  midi_msg_t* midi_ptr = (midi_msg_t*)llist_pop_front(&midi_nodes_pool);  // Get a node from the MIDI nodes stack
  //midi_ptr = (midi_msg_t*)midiMsg;
  
  midi_ptr->type = midiMsg.type;       // Set the MIDI type
  midi_ptr->data1 = midiMsg.data1;     // Set the MIDI note
  midi_ptr->data2 = midiMsg.data2;     // Set the MIDI velocity
  midi_ptr->channel = midiMsg.channel; // Set the MIDI channel
  
  #if defined(MIDI_THRU)
    llist_push_front(&midi_out, midi_ptr);  // Add the node to the midi_out linked list
  #else
    llist_push_front(&midi_in, midi_ptr);   // Add the node to the midi_in linked list
  #endif
};

void hardware_midi_transmit(void) {
  for (lnode_t* midi_node_ptr = ITERATOR_START_FROM_HEAD(&midi_out); midi_node_ptr != NULL; midi_node_ptr = ITERATOR_NEXT(midi_node_ptr)) {
    midi_msg_t* midi_ptr = (midi_msg_t*)ITERATOR_DATA(midi_node_ptr);
    MIDI.send(midi_ptr->type, midi_ptr->data1, midi_ptr->data2, midi_ptr->channel);
  };
  llist_concat_nodes(&midi_nodes_pool, &midi_out); // Save/rescure all midi_out nodes
};
