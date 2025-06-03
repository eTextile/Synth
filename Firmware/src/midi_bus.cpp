/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "blob.h"
#include "mapping.h"

#define MIDI_NODES 128

midi_msg_t midi_nodes_array[MIDI_NODES] = {}; // Memory allocation for all MIDI I/O messages

llist_t midi_nodes_pool; // Main MIDI node stack
llist_t midi_in;         // Main MIDI input linked list
llist_t midi_out;        // Main MIDI output linked list
//llist_t midi_chord;      // Main MIDI chord linked list

void midi_bus_setup(void) {
  llist_builder(&midi_nodes_pool, &midi_nodes_array[0], MIDI_NODES, sizeof(midi_nodes_array[0])); // Add X nodes to the midi_nodes_pool
  llist_raz(&midi_in);
  llist_raz(&midi_out);
  //llist_raz(&midi_chord);
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
  if (node_ptr) {
    node_ptr->type = midi_ptr->type;        // Set the MIDI type
    node_ptr->data1 = midi_ptr->data1;      // Set the MIDI note/cc/...
    node_ptr->data2 = midi_ptr->data2;      // Set the MIDI velocity
    node_ptr->channel = midi_ptr->channel;  // Set the MIDI channel
    llist_push_front(&midi_out, node_ptr);  // Add the node to the midi_out linked list
  }
  else {
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LLIST)
      Serial.printf("\nNo more nodes left in the : midi_nodes_pool -> see midi_send_out()");
    #endif
    set_mode(ERROR_MODE);
  }
};

void mapping_send_midi_note_on(positon_t* positon_ptr, blob_t* blob_ptr) {
  positon_ptr->msg.type = NoteOn;
  positon_ptr->msg.data2 = blob_ptr->centroid.x; // TODO: improve "velocity" sensing
  midi_send_out(&positon_ptr->msg);
};

void mapping_send_midi_note_off(positon_t* positon_ptr, blob_t* blob_ptr) {
  positon_ptr->msg.type = NoteOff;
  positon_ptr->msg.data2 = 0;
  midi_send_out(&positon_ptr->msg);
};

void mapping_send_midi_pos_x_msg(rect_t* bounding_box_ptr, positon_t* positon_ptr, blob_t* blob_ptr) {
  positon_ptr->last_val = positon_ptr->msg.data2;
  positon_ptr->msg.data2 = map(
    blob_ptr->centroid.x,
    bounding_box_ptr->from.x,
    bounding_box_ptr->to.x,
    positon_ptr->limit.min,
    positon_ptr->limit.max
  );
  if (positon_ptr->msg.data2 != positon_ptr->last_val) {
    midi_send_out(&positon_ptr->msg);
  }
};

void mapping_send_midi_pos_y_msg(rect_t* bounding_box_ptr, positon_t* positon_ptr, blob_t* blob_ptr) {
  positon_ptr->last_val = positon_ptr->msg.data2;
  positon_ptr->msg.data2 = map(
    blob_ptr->centroid.y,
    bounding_box_ptr->from.y,
    bounding_box_ptr->to.y,
    positon_ptr->limit.min,
    positon_ptr->limit.max
  );
  if (positon_ptr->msg.data2 != positon_ptr->last_val) {
    midi_send_out(&positon_ptr->msg);
  }
};

void mapping_send_midi_pos_z_msg(positon_t* positon_ptr, blob_t* blob_ptr) {
  positon_ptr->last_val = positon_ptr->msg.data2;
  positon_ptr->msg.data2 = map(
    blob_ptr->centroid.z,
    Z_MIN,
    Z_MAX,
    positon_ptr->limit.min,
    positon_ptr->limit.max
  );
  if (positon_ptr->msg.data2 != positon_ptr->last_val) {
    midi_send_out(&positon_ptr->msg);
  }
};

/*
void midi_handle_input(const Message<128u> &midiMsg) {
  midi_msg_t* node_ptr = (midi_msg_t*)llist_pop_front(&midi_nodes_pool);
  if (node_ptr) {
    node_ptr-type = midiMsg.type;         // Set the MIDI type
    node_ptr->data1 = midiMsg.data1;      // Set the MIDI note
    node_ptr->data2 = midiMsg.data2;      // Set the MIDI velocity
    node_ptr->channel = midiMsg.channel;  // Set the MIDI channel
    llist_push_front(&midi_in, node_ptr); // Add the node to the midi_in linked list
    }
    else {
      #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LLIST)
        Serial.printf("\nNo more nodes left in the : midi_nodes_pool -> see midi_handle_input()");
      #endif
      set_mode(ERROR_MODE);
    }
  }
};
*/

const char* get_type_name(MidiType code) {
  const char* char_code = NULL;
  switch (code) {
    case NoteOn: char_code = "NOTE_ON"; break;
    case NoteOff: char_code = "NOTE_OFF"; break;
    case ControlChange: char_code = "C_CHANGE"; break;
    case AfterTouchPoly: char_code = "P_AFTERTOUCHPOLY"; break;
    case PitchBend: char_code = "PITCH_BEND"; break;
    default:
      break;
  }
  return char_code;
};

void print_bytes(const uint8_t* data_ptr, size_t data_length) {
  Serial.printf("\nPRINT_BYTES / DATA_LENGTH: %d", data_length);
  Serial.printf("\nPRINT_BYTES / DATA: ");
  for (size_t i = 0; i < data_length; i++) {
    Serial.printf("%c", char(*data_ptr));
    data_ptr++;
  }
};
