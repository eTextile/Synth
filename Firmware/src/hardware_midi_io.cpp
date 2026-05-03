/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "hardware_midi_io.h"

//#include "config.h"
//#include "llist.h"
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

// Handle hardware input midi msg
// midi_msg struct is C++ and this project is C : can it be refact for zero-copy ?
void hardware_midi_handle_input(const Message<128u> &midi_msg) {

  midi_msg_t* midi_msg_ptr = (midi_msg_t*)llist_pop_front(&llist_midi_nodes_pool);

  if (midi_msg_ptr != NULL) {

    midi_msg_ptr->channel = midi_msg.channel;
    midi_msg_ptr->type = midi_msg.type;
    midi_msg_ptr->data1 = midi_msg.data1;
    midi_msg_ptr->data2 = midi_msg.data2;

    switch (midi_msg_ptr->type) {

    case midi::NoteOn:
      for (lnode_t* mapping_node_ptr = ITERATOR_START_FROM_HEAD(&llist_mappings); mapping_node_ptr != NULL; mapping_node_ptr = ITERATOR_NEXT(mapping_node_ptr)) {
        common_t* mapping_ptr = (common_t*)ITERATOR_DATA(mapping_node_ptr);
        if (mapping_ptr->midi_hardware_receive_func_ptr(mapping_ptr, midi_msg_ptr)) {
          mapping_ptr->midi_hardware_update_func_ptr(mapping_ptr, midi_msg_ptr);
          break;
        }
      }
      break;

    case midi::NoteOff:
      for (lnode_t* mapping_node_ptr = ITERATOR_START_FROM_HEAD(&llist_mappings); mapping_node_ptr != NULL; mapping_node_ptr = ITERATOR_NEXT(mapping_node_ptr)) {
        common_t* mapping_ptr = (common_t*)ITERATOR_DATA(mapping_node_ptr);
        if (mapping_ptr->midi_hardware_receive_func_ptr(mapping_ptr, midi_msg_ptr)) {
          mapping_ptr->midi_hardware_dispose_func_ptr(mapping_ptr, midi_msg_ptr);
          break;
        }
      }
      break;

    default:
      break;
    }

  }
};

// Send a MIDI Real Time TimingClock (0xF8) on the hardware MIDI DIN port — called by tap_tempo_clock_tick().
void hardware_midi_send_clock(void) {
  MIDI.sendRealTime(midi::TimingClock);
};

void hardware_midi_transmit_mappings_midi_msg(void) {
  for (lnode_t* midi_node_ptr = ITERATOR_START_FROM_HEAD(&llist_midi_out); midi_node_ptr != NULL; midi_node_ptr = ITERATOR_NEXT(midi_node_ptr)) {
    midi_msg_t* midi_msg_ptr = (midi_msg_t*)ITERATOR_DATA(midi_node_ptr);
    MIDI.send(midi_msg_ptr->type, midi_msg_ptr->data1, midi_msg_ptr->data2, midi_msg_ptr->channel);
  }
  while (MIDI.read()); // Read and discard any incoming MIDI messages
};
