/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "hardware_midi_io.h"
#include "mapping.h"

// Teensy 4.0 : Serial3 / MIDI_OUT -> PIN 14 / MIDI_IN -> PIN 15
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, HARDWARE_MIDI);

static void hardware_midi_read_note_on(byte, byte, byte);
static void hardware_midi_read_note_off(byte, byte, byte);

// ─── SETUP ────────────────────────────────────────────────────────────────────

void hardware_midi_setup(void) {
  HARDWARE_MIDI.begin(MIDI_CHANNEL_OMNI);
  HARDWARE_MIDI.turnThruOff();
  HARDWARE_MIDI.setHandleNoteOn(hardware_midi_read_note_on);
  HARDWARE_MIDI.setHandleNoteOff(hardware_midi_read_note_off);
};

// ─── INPUT ────────────────────────────────────────────────────────────────────

// Forward a hardware MIDI IN message to the USB host (PLAY mode only).
static void hardware_midi_forward_input(midi_msg_t* msg) {
  usbMIDI.send((uint8_t)msg->type, msg->data1, msg->data2, msg->channel, 0);
  while (usbMIDI.read());
};

static void hardware_midi_read_note_on(byte channel, byte note, byte velocity) {
  // midi_msg_t msg = { (uint8_t)channel, NoteOn, note, velocity };
  midi_msg_t msg;
  
  msg.channel = (uint8_t)channel;
  msg.type = NoteOn;
  msg.data1 = (uint8_t)note;
  msg.data2 = (uint8_t)velocity;

  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_mappings); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    common_t* mapping_ptr = (common_t*)ITERATOR_DATA(node_ptr);
    if (mapping_ptr->hardware_midi_receive_func_ptr(mapping_ptr, &msg)) {
      midi_msg_t* stored = (midi_msg_t*)llist_pop_front(&llist_midi_nodes_pool);
      if (stored != NULL) {
        *stored = msg;
        mapping_ptr->hardware_midi_update_func_ptr(mapping_ptr, stored);
      }
      break;
    }
  }
  if (e256_current_mode == PLAY_MODE) hardware_midi_forward_input(&msg);
  
};

static void hardware_midi_read_note_off(byte channel, byte note, byte velocity) {
  // midi_msg_t msg = { (uint8_t)channel, NoteOff, note, velocity };
  midi_msg_t msg;
  
  msg.channel = (uint8_t)channel;
  msg.type = NoteOff;
  msg.data1 = (uint8_t)note;
  msg.data2 = (uint8_t)velocity;


  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_mappings); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    common_t* mapping_ptr = (common_t*)ITERATOR_DATA(node_ptr);
    if (mapping_ptr->hardware_midi_receive_func_ptr(mapping_ptr, &msg)) {
      mapping_ptr->hardware_midi_dispose_func_ptr(mapping_ptr, &msg);
      break;
    }
  }
  if (e256_current_mode == PLAY_MODE) hardware_midi_forward_input(&msg);
};

void hardware_midi_receive(void) {
  while (HARDWARE_MIDI.read());
};

// ─── OUTPUT ───────────────────────────────────────────────────────────────────

// Send a MIDI Real Time TimingClock (0xF8) on the hardware mini jack TRS Type-A port.
void hardware_midi_send_clock(void) {
  HARDWARE_MIDI.sendClock();
};

void mapping_hardware_midi_transmit(void) {
  for (lnode_t* midi_node_ptr = ITERATOR_START_FROM_HEAD(&llist_midi_out); midi_node_ptr != NULL; midi_node_ptr = ITERATOR_NEXT(midi_node_ptr)) {
    midi_msg_t* midi_msg_ptr = (midi_msg_t*)ITERATOR_DATA(midi_node_ptr);
    HARDWARE_MIDI.send(midi_msg_ptr->type, midi_msg_ptr->data1, midi_msg_ptr->data2, midi_msg_ptr->channel);
  }
  while (HARDWARE_MIDI.read());
};
