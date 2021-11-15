/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "hardware_midi_transmit.h"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);

void HARDWARE_MIDI_TRANSMIT_SETUP(void) {
  MIDI.begin(MIDI_INPUT_CHANNEL); // Launch MIDI and listen to channel 1
  MIDI.setHandleMessage(hardware_midi_handle_input);
};

void hardware_midi_read_input(void) {
  MIDI.read(MIDI_INPUT_CHANNEL);            // Is there a MIDI incoming messages on channel One
  while (MIDI.read(MIDI_INPUT_CHANNEL));    // Read and discard any incoming MIDI messages
};

void hardware_midi_handle_input(const midi::Message<128u> &midiMsg) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  switch (midiMsg.type) {
    case midi::NoteOn:
      node_ptr->midiMsg.status = midi::NoteOn;         // Set the MIDI status
      node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI note
      node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI velocity
      //node_ptr->midiMsg.channel = midiMsg.channel;   // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    case midi::NoteOff:
      node_ptr->midiMsg.status = midi::NoteOff;        // Set the MIDI status
      node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI note
      node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI velocity
      //node_ptr->midiMsg.channel = midiMsg.channel;   // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    case midi::ControlChange:
      node_ptr->midiMsg.status = midi::ControlChange;  // Set the MIDI status
      node_ptr->midiMsg.data1 = midiMsg.data1;         // Set the MIDI control
      node_ptr->midiMsg.data2 = midiMsg.data2;         // Set the MIDI value
      //node_ptr->midiMsg.channel = midiMsg.channel;   // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    case midi::Clock:
      node_ptr->midiMsg.status = midi::Clock;          // Set the MIDI status
      //node_ptr->midiMsg.data1 = midiMsg.data1;       // Set the MIDI note
      //node_ptr->midiMsg.data2 = midiMsg.data2;       // Set the MIDI velocity
      //node_ptr->midiMsg.channel = midiMsg.channel;   // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    default:
      llist_push_front(&midi_node_stack, node_ptr);    // Add the node to the midi_node_stack linked liste
      break;
  };
};

void hardware_midi_transmit(void) {
  for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
    switch (currentMode) {
      case MAPPING_LIB:
        for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
        case midi::NoteOn:
          MIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);     // Hardware send MIDI noteOn#endif
        case midi::NoteOff:
          MIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);     // Hardware send MIDI noteOff
          break;
        case midi::ControlChange:
          MIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);    // Hardware send MIDI control_change
          break;
        default:
          break;
        };
    };
  };
};