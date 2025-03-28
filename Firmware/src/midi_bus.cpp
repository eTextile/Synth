/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "midi_bus.h"
#include "blob.h"

#define MIDI_NODES 128

midi_msg_t midi_nodes_array[MIDI_NODES] = {}; // Memory allocation for all MIDI I/O messages

llist_t midi_nodes_pool; // Main MIDI node stack
llist_t midi_in;         // Main MIDI input linked list
llist_t midi_out;        // Main MIDI output linked list
llist_t midi_chord;      // Main MIDI chord linked list

void midi_bus_setup(void) {
  llist_builder(&midi_nodes_pool, &midi_nodes_array[0], MIDI_NODES, sizeof(midi_nodes_array[0])); // Add X nodes to the midi_nodes_pool
  llist_raz(&midi_in);
  llist_raz(&midi_out);
  llist_raz(&midi_chord);
};

// Extract MIDI type and channel from MIDI status msg
// https://www.midi.org/specifications-old/item/table-2-expanded-messages-list-status-bytes
void midi_msg_status_unpack(uint8_t in_status, midi_status_t* out_status) {
  out_status->type = (MidiType)(in_status & 0xF0); // Extract the midi-type from the status byte
  out_status->channel = (in_status & 0xF) + 1; // Extract the midi-channel from the status byte
};

// Concatenate MIDI status msg from MIDI type and channel
uint8_t midi_msg_status_pack(MidiType type, uint8_t channel) {
  uint8_t status = (channel - 1) | (uint8_t)type;
  return status;
};

void midi_send_out(midi_msg_t* midi_ptr) {
  midi_msg_t* node_ptr = (midi_msg_t*)llist_pop_front(&midi_nodes_pool);  // Get a node from the MIDI nodes stack
  if (!node_ptr) Serial.printf("\nNo more nodes left in the pool");
  node_ptr->type = midi_ptr->type;        // Set the MIDI type
  node_ptr->data1 = midi_ptr->data1;      // Set the MIDI note/cc/...
  node_ptr->data2 = midi_ptr->data2;      // Set the MIDI velocity
  node_ptr->channel = midi_ptr->channel;  // Set the MIDI channel
  llist_push_front(&midi_out, node_ptr);  // Add the node to the midi_out linked list
};


/*
void midi_handle_input(const Message<128u> &midiMsg) {
  midi_msg_t* node_ptr = (midi_msg_t*)llist_pop_front(&midi_nodes_pool);  // Get a node from the MIDI nodes stack
  node_ptr-type = midiMsg.type;         // Set the MIDI type
  node_ptr->data1 = midiMsg.data1;      // Set the MIDI note
  node_ptr->data2 = midiMsg.data2;      // Set the MIDI velocity
  node_ptr->channel = midiMsg.channel;  // Set the MIDI channel
  llist_push_front(&midi_in, node_ptr); // Add the node to the midi_in linked list    
};
*/

void print_bytes(const uint8_t* data_ptr, size_t data_length) {
  Serial.printf("\nPRINT_BYTES / DATA_LENGTH: %d", data_length);
  Serial.printf("\nPRINT_BYTES / DATA: ");
  for (size_t i = 0; i < data_length; i++) {
    Serial.printf("%c", char(*data_ptr));
    data_ptr++;
  }
};
