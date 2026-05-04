/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "hardware_midi_io.h"
#include "mapping.h"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);

uint8_t hardware_midi_input_channel = MIDI_INPUT_CHANNEL;

void hardware_midi_setup(void) {
  MIDI.begin(hardware_midi_input_channel);
  MIDI.setHandleMessage(hardware_midi_handle_input);
};

void hardware_midi_set_input_channel(uint8_t channel) {
  hardware_midi_input_channel = channel;
  MIDI.begin(channel);
};

void hardware_midi_receive(void) {
  MIDI.read(hardware_midi_input_channel);
  while (MIDI.read(hardware_midi_input_channel));
};

void hardware_midi_handle_input(const Message<128u> &lib_msg) {
  // Stack-allocated — no pool touch until we know a mapping actually matches.
  midi_msg_t msg = { lib_msg.channel, lib_msg.type, lib_msg.data1, lib_msg.data2 };

  switch (msg.type) {

  case midi::NoteOn:
    for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_mappings); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
      common_t* mapping_ptr = (common_t*)ITERATOR_DATA(node_ptr);
      // receive() only reads msg.channel — safe to pass a stack pointer.
      if (mapping_ptr->midi_hardware_receive_func_ptr(mapping_ptr, &msg)) {
        // Allocate only here: update() stores the pointer in the mapping's llist.
        midi_msg_t* stored = (midi_msg_t*)llist_pop_front(&llist_midi_nodes_pool);
        if (stored != NULL) {
          *stored = msg;
          mapping_ptr->midi_hardware_update_func_ptr(mapping_ptr, stored);
        }
        break;
      }
    }
    break;

  case midi::NoteOff:
    for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_mappings); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
      common_t* mapping_ptr = (common_t*)ITERATOR_DATA(node_ptr);
      if (mapping_ptr->midi_hardware_receive_func_ptr(mapping_ptr, &msg)) {
        // dispose() pops from the mapping's own llist and does not store msg — stack pointer is safe.
        mapping_ptr->midi_hardware_dispose_func_ptr(mapping_ptr, &msg);
        break;
      }
    }
    break;

  default:
    break;
  }
};

// Send a MIDI Real Time TimingClock (0xF8) on the hardware MIDI DIN port — called by tap_tempo_clock_tick().
void hardware_midi_send_clock(void) {
  MIDI.sendClock();
};

void mapping_hardware_midi_transmit(void) {
  for (lnode_t* midi_node_ptr = ITERATOR_START_FROM_HEAD(&llist_midi_out); midi_node_ptr != NULL; midi_node_ptr = ITERATOR_NEXT(midi_node_ptr)) {
    midi_msg_t* midi_msg_ptr = (midi_msg_t*)ITERATOR_DATA(midi_node_ptr);
    MIDI.send(midi_msg_ptr->type, midi_msg_ptr->data1, midi_msg_ptr->data2, midi_msg_ptr->channel);
  }
  while (MIDI.read()); // Read and discard any incoming MIDI messages
};
