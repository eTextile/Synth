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

/*
void hardware_midi_recive(void) {
  MIDI.read(MIDI_INPUT_CHANNEL);         // Is there any incoming MIDI messages on channel 1
  while (MIDI.read(MIDI_INPUT_CHANNEL)); // Read and discard any incoming MIDI messages
};
*/

void hardware_midi_handle_input(const Message<128u> &midiMsg) {
  // midiMsg struct is C++
  // Can it be refact for zero-copy ?
  midi_msg_t* midi_ptr = (midi_msg_t*)llist_pop_front(&midi_nodes_pool);  // Get a node from the MIDI nodes stack  
  if (midi_ptr) {
    midi_ptr->type = midiMsg.type;
    midi_ptr->data1 = midiMsg.data1;
    midi_ptr->data2 = midiMsg.data2;
    midi_ptr->channel = midiMsg.channel;
    #if defined(MIDI_THROUGH)
      llist_push_front(&midi_out, midi_ptr);
    #else
      llist_push_front(&midi_in, midi_ptr);
    #endif
  }
  else {
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LLIST)
      Serial.printf("\nNo more nodes left in the : midi_nodes_pool -> see hardware_midi_handle_input()");
    #endif
    set_mode(ERROR_MODE);
  }
};

void hardware_midi_transmit_mappings_midi_msg(void) {
  for (lnode_t* midi_node_ptr = ITERATOR_START_FROM_HEAD(&midi_out); midi_node_ptr != NULL; midi_node_ptr = ITERATOR_NEXT(midi_node_ptr)) {
    midi_msg_t* midi_ptr = (midi_msg_t*)ITERATOR_DATA(midi_node_ptr);
    MIDI.send(midi_ptr->type, midi_ptr->data1, midi_ptr->data2, midi_ptr->channel);
  }
  while (MIDI.read()); // Read and discard any incoming MIDI messages
};
