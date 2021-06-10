/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.

  What about the TUIO 1.1 Protocol Specification
  http://www.tuio.org/?specification
  What about the MIDI_MPE Protocol Specification
  https://www.midi.org/midi-articles/midi-polyphonic-expression-mpe
*/
//#if HARDWARE_MIDI

#include "hardware_midi_transmit.h"

extern preset_t presets;

midiNode_t midiInArray[MAX_SYNTH] = {0}; // 1D Array to alocate memory for incoming midi notes
llist_t  midiNodes_stack;                // Midi nodes stack

void midi_llist_init(llist_t* nodes_ptr, midiNode_t* nodeArray_ptr) {
  llist_raz(nodes_ptr);
  for (int i = 0; i < MAX_SYNTH; i++) {
    llist_push_front(nodes_ptr, &nodeArray_ptr[i]);
  };
};

void HARDWARE_MIDI_SETUP(void) {
  midi_llist_init(&midiNodes_stack, &midiInArray[0]);
  MIDI.begin(MIDI_CHANNEL_OMNI);
};

boolean midi_handle_hardware_input(llist_t* llist_ptr) {
  if (MIDI.read()) {                   // If incoming MIDI message
    byte type = MIDI.getType();        // Get the type of the MIDI message
    switch (type) {
      case midi::NoteOn:
        midiNode_t* midiNode = (midiNode_t*)llist_pop_front(&midiNodes_stack);
        midiNode->pithch = MIDI.getData1();
        midiNode->velocity = MIDI.getData2();
        midiNode->channel = MIDI.getChannel();
        llist_push_front(llist_ptr, midiNode);
        break;
      case midi::NoteOff:
        midiNode_t* prevNode_ptr = NULL;
        for (midiNode_t* midiNode = (midiNode_t*)ITERATOR_START_FROM_HEAD(llist_ptr); midiNode != NULL; midiNode = (midiNode_t*)ITERATOR_NEXT(midiNode)) {
          if (midiNode->pithch == MIDI.getData1()) {
            llist_extract_node(llist_ptr, prevNode_ptr, midiNode);
            llist_push_front(&midiNodes_stack, midiNode);
            break;
          };
          prevNode_ptr = midiNode;
        };
        break;
      default:
        break;
    };
    return true;
  }
  else {
    return false;
  };
};

// Send all blobs values using ControlChange MIDI format
void midi_send_blobs(llist_t* llist_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    MIDI.sendControlChange(BS, blob_ptr->state, blob_ptr->UID + 1);
    MIDI.sendControlChange(BL, blob_ptr->lastState, blob_ptr->UID + 1);
    MIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, X_MAX, 0, 127)), blob_ptr->UID + 1);
    MIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, Y_MAX, 0, 127)), blob_ptr->UID + 1);
    MIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
    MIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
    MIDI.sendControlChange(BD, constrain(blob_ptr->box.D, 0, 127), blob_ptr->UID + 1);
  }
  //while (MIDI.read()); // Read and discard any incoming MIDI messages
}

// ccPesets_ptr -> ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]
void midi_control_change(llist_t* llist_ptr, ccPesets_t* ccPesets_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    // Test if we are within the blob limit
    if (blob_ptr->UID == ccPesets_ptr->blobID) {
      // Test if the blob is alive
      if (blob_ptr->state) {
        switch (ccPesets_ptr->mappVal) {
          case BX:
            if (blob_ptr->centroid.X != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.X;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.X, 0, 127), ccPesets_ptr->midiChannel);
#else
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.X, 0, 127), ccPesets_ptr->midiChannel);
#endif
            }
            break;
          case BY:
            if (blob_ptr->centroid.Y != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Y;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.Y, 0, 127), ccPesets_ptr->midiChannel);
#else
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.Y, 0, 127), ccPesets_ptr->midiChannel);
#endif
            }
            break;
          case BW:
            if (blob_ptr->box.W != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.W;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.W, 0, 127), ccPesets_ptr->midiChannel);
#else
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.W, 0, 127), ccPesets_ptr->midiChannel);
#endif
            }
            break;
          case BH:
            if (blob_ptr->box.H != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.H;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.H, 0, 127), ccPesets_ptr->midiChannel);
#else
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.H, 0, 127), ccPesets_ptr->midiChannel);
#endif
            }
            break;
          case BD:
            if (blob_ptr->box.D != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.D;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nBLOB:%d\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, ccPesets_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), ccPesets_ptr->midiChannel);
#else
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), ccPesets_ptr->midiChannel);
#endif
            }
            break;
        }
      }
    }
  }
}
//#endif
