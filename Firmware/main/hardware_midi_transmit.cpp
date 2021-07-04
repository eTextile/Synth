/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.

  What about the TUIO 1.1 Protocol Specification
  http://www.tuio.org/?specification
  What about the MIDI_MPE Protocol Specification
  https://www.midi.org/midi-articles/midi-polyphonic-expression-mpe
*/

#include "hardware_midi_transmit.h"

#if HARDWARE_MIDI_TRANSMIT

MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);

midiNode_t midiNodesArray[MAX_SYNTH] = {0}; // 1D Array to alocate memory for incoming midi notes
llist_t midiNodesStack;                     // Midi nodes stack

void hardware_midi_llist_init(llist_t* nodes_ptr, midiNode_t* nodeArray_ptr, const int nodes) {
  llist_raz(nodes_ptr);
  for (int i = 0; i < nodes; i++) {
    llist_push_front(nodes_ptr, &nodeArray_ptr[i]);
  };
};

void HARDWARE_MIDI_SETUP(void) {
  hardware_midi_llist_init(&midiNodesStack, &midiNodesArray[0], MAX_SYNTH);
  MIDI.begin(MIDI_CHANNEL_OMNI);
};

void hardware_midi_handle_input(void) {
  if (MIDI.read()) {                   // If incoming MIDI message
    byte type = MIDI.getType();        // Get the type of the MIDI message
    switch (type) {
      case midi::NoteOn:
        midiNode_t* midiNode = (midiNode_t*)llist_pop_front(&midiNodesStack);
        midiNode->pithch = MIDI.getData1();
        midiNode->velocity = MIDI.getData2();
        midiNode->channel = MIDI.getChannel();
        llist_push_front(&midiIn, midiNode);
        break;
      case midi::NoteOff:
        midiNode_t* prevNode_ptr = NULL;
        for (midiNode_t* midiNode = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiIn); midiNode != NULL; midiNode = (midiNode_t*)ITERATOR_NEXT(midiNode)) {
          if (midiNode->pithch == MIDI.getData1()) {
            llist_extract_node(&midiIn, prevNode_ptr, midiNode);
            llist_push_front(&midiNodesStack, midiNode);
            break;
          };
          prevNode_ptr = midiNode;
        };
        break;
      default:
        break;
    };
  }
  else {
  };
};

// Send all blobs values using MIDI format
void hardware_midi_send_blobs(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->state) {
      if (!blob_ptr->lastState) MIDI.sendNoteOn(blob_ptr->UID + 1, 1, 0);
      MIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1); // Make it Q2
      MIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
      MIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
      MIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
      MIDI.sendControlChange(BD, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
      /*
        usbMIDI.sendAfterTouchPoly(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
        usbMIDI.sendAfterTouchPoly(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
        usbMIDI.sendAfterTouchPoly(BW, blob_ptr->box.W, blob_ptr->UID + 1);
        usbMIDI.sendAfterTouchPoly(BH, blob_ptr->box.H, blob_ptr->UID + 1);
        usbMIDI.sendAfterTouchPoly(BD, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
      */
    }
    else {
      MIDI.sendNoteOff(blob_ptr->UID + 1, 0, 0);
    };
  };
  while (MIDI.read()); // Read and discard any incoming MIDI messages
};
#endif
