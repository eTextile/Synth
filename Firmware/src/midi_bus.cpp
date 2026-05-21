/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

#define MIDI_NODES 127

midi_msg_t midi_nodes_array[MIDI_NODES] = {}; // Memory allocation for all MIDI I/O messages

llist_t llist_midi_nodes_pool;   // Main MIDI node stack
llist_t llist_midi_in;           // Main MIDI input linked list
llist_t llist_midi_out;          // Main MIDI output linked list

void midi_bus_setup(void) {
  llist_builder(&llist_midi_nodes_pool, &midi_nodes_array[0], MIDI_NODES, sizeof(midi_nodes_array[0])); // Add X nodes to the llist_midi_nodes_pool
  llist_raz(&llist_midi_in);
  llist_raz(&llist_midi_out);
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

// Chord intervals indexed by type (1–9), matching web app CHORD_NAMES.
// Rows: up to 4 semitone offsets from root; unused slots filled with 0.
static const uint8_t CHORD_INTERVALS[10][MAX_CHORD_NOTES] = {
  {0,  0,  0,  0},  // [0] unused
  {0,  4,  7,  0},  // [1] Major
  {0,  3,  7,  0},  // [2] Minor
  {0,  3,  6,  0},  // [3] Diminished
  {0,  4,  8,  0},  // [4] Augmented
  {0,  4,  7, 11},  // [5] Maj7
  {0,  3,  7, 10},  // [6] Min7
  {0,  4,  7, 10},  // [7] Dom7
  {0,  2,  7,  0},  // [8] Sus2
  {0,  5,  7,  0},  // [9] Sus4
};
static const uint8_t CHORD_NOTE_COUNT[10] = {0, 3, 3, 3, 3, 4, 4, 4, 3, 3};

// chord_msgs: pre-allocated array of MAX_CHORD_NOTES midi_msg_t embedded in the mapping struct.
void midi_send_chord_on(midi_msg_t* chord_msgs, const midi_chord_t* chord, uint8_t channel, uint8_t velocity) {
  uint8_t t = (chord->type >= 1 && chord->type <= 9) ? chord->type : 1;
  uint8_t n = CHORD_NOTE_COUNT[t];
  for (uint8_t j = 0; j < n; j++) {
    chord_msgs[j].type    = NoteOn;
    chord_msgs[j].channel = channel;
    chord_msgs[j].data1   = (uint8_t)constrain(chord->note + CHORD_INTERVALS[t][j], 0, 127);
    chord_msgs[j].data2   = velocity;
    llist_push_front(&llist_midi_out, &chord_msgs[j]);
  }
}

void midi_send_chord_off(midi_msg_t* chord_msgs, uint8_t chord_type) {
  uint8_t t = (chord_type >= 1 && chord_type <= 9) ? chord_type : 1;
  uint8_t n = CHORD_NOTE_COUNT[t];
  for (uint8_t j = 0; j < n; j++) {
    chord_msgs[j].type  = NoteOff;
    chord_msgs[j].data2 = 0;
    llist_push_front(&llist_midi_out, &chord_msgs[j]);
  }
}

void print_bytes(const uint8_t* data_ptr, size_t data_length) {
  Serial.printf("\nPRINT_BYTES / DATA_LENGTH: %d", data_length);
  Serial.printf("\nPRINT_BYTES / DATA: ");
  for (size_t i = 0; i < data_length; i++) {
    Serial.printf("%c", char(*data_ptr));
    data_ptr++;
  }
};
