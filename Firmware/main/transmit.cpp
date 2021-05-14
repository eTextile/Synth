/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.

  What about the TUIO 1.1 Protocol Specification
  http://www.tuio.org/?specification
  What about the MIDI_MPE Protocol Specification
  https://www.midi.org/midi-articles/midi-polyphonic-expression-mpe
*/

#include "transmit.h"

midiNode_t midiInArray[MAX_SYNTH] = {0}; // 1D Array to alocate memory for incoming midi notes
llist_t  midiNodes;                      // Midi nodes stack

#if USB_SLIP_OSC
SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);

void USB_SLIP_OSC_SETUP(void) {
  SLIPSerial.begin(BAUD_RATE); // FIXME
}

void usb_slipOsc(llist_t* llist_ptr) {
  OSCBundle OSCbundle;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    OSCMessage msg("/b");
    msg.add(blob_ptr->UID);
    msg.add(blob_ptr->state);
    msg.add(blob_ptr->centroid.X);
    msg.add(blob_ptr->centroid.Y);
    msg.add(blob_ptr->box.W);
    msg.add(blob_ptr->box.H);
    msg.add(blob_ptr->box.D);
    OSCbundle.add(msg);
  }
  SLIPSerial.beginPacket();     // Send SLIP header
  OSCbundle.send(SLIPSerial);   // Send the OSC bundle
  SLIPSerial.endPacket();       // Send the SLIP end of packet
}
#endif

#if HARDWARE_MIDI
void midi_llist_init(llist_t* nodes_ptr, midiNode_t* nodeArray_ptr) {
  llist_raz(nodes_ptr);
  for (int i = 0; i < MAX_SYNTH; i++) {
    llist_push_front(nodes_ptr, &nodeArray_ptr[i]);
  }
}

void HARDWARE_MIDI_SETUP(void) {
  midi_llist_init(&midiNodes, &midiInArray[0]);
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

boolean handleMidiInput(llist_t* llist_ptr) {
  if (MIDI.read()) {                   // Is there a MIDI message incoming
    byte type = MIDI.getType();        // Get the type of the message we caught
    switch (type) {
      case midi::NoteOn:
        midiNode_t* midiNode = (midiNode_t*)llist_pop_front(&midiNodes);
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
            llist_push_front(&midiNodes, midiNode);
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
#endif

//////////////////////////////////////////////////////
#if USB_MIDI
void USB_MIDI_SETUP(void) {
  usbMIDI.begin();
}

// Send blobs values using ControlChange MIDI format
// Send only the last blob that have been added to the sensor surface
// Separate blob's values according to the encoder position to allow the mapping into Max4Live
void usb_midi_learn(llist_t* llist_ptr, preset_t* preset_ptr) {
  blob_t* lastBlob_ptr = (blob_t*)llist_ptr->tail_ptr;
  switch (preset_ptr->val) {
    case BS:
      usbMIDI.sendControlChange(BS, lastBlob_ptr->state, lastBlob_ptr->UID + 1);
      break;
    case BX:
      usbMIDI.sendControlChange(BX, (uint8_t)round(map(lastBlob_ptr->centroid.X, 0.0, 59.0, 0, 127)), lastBlob_ptr->UID + 1);
      break;
    case BY:
      usbMIDI.sendControlChange(BY, (uint8_t)round(map(lastBlob_ptr->centroid.Y, 0.0, 59.0, 0, 127)), lastBlob_ptr->UID + 1);
      break;
    case BW:
      usbMIDI.sendControlChange(BW, lastBlob_ptr->box.W, lastBlob_ptr->UID + 1);
      break;
    case BH:
      usbMIDI.sendControlChange(BH, lastBlob_ptr->box.H, lastBlob_ptr->UID + 1);
      break;
    case BD:
      usbMIDI.sendControlChange(BD, constrain(lastBlob_ptr->box.D, 0, 127), lastBlob_ptr->UID + 1);
      break;
  }
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
}

// Send all blobs values using ControlChange MIDI format
void usb_midi_play(llist_t* llist_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    usbMIDI.sendControlChange(BS, blob_ptr->state, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, 59.0, 0, 127)), blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, 59.0, 0, 127)), blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BD, constrain(blob_ptr->box.D, 0, 127), blob_ptr->UID + 1);
  }
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
}
#endif

// ccPesets_ptr -> ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]
void controlChange(llist_t* llist_ptr, ccPesets_t* ccPesets_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    // Test if we are within the blob limit
    if (blob_ptr->UID == ccPesets_ptr->blobID) {
      // Test if the blob is alive
      if (blob_ptr->state) {
#if HARDWARE_MIDI
        switch (ccPesets_ptr->mappVal) {
          case BX:
            if (blob_ptr->centroid.X != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.X;
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.X, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BY:
            if (blob_ptr->centroid.Y != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Y;
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.Y, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BW:
            if (blob_ptr->box.W != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.W;
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.W, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BH:
            if (blob_ptr->box.H != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.H;
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.H, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BD:
            if (blob_ptr->box.D != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.D;
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
        }
#endif
#if USB_MIDI
        switch (ccPesets_ptr->mappVal) {
          case BX:
            if (blob_ptr->centroid.X != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.X;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.X, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BY:
            if (blob_ptr->centroid.Y != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Y;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.Y, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BW:
            if (blob_ptr->box.W != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.W;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.W, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BH:
            if (blob_ptr->box.H != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.H;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.H, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BD:
            if (blob_ptr->box.D != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.D;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
        }
#endif
#if DEBUG_MAPPING
        switch (ccPesets_ptr->mappVal) {
          case BX:
            if (blob_ptr->centroid.X != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.X;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.X, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BY:
            if (blob_ptr->centroid.Y != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Y;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.Y, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BW:
            if (blob_ptr->box.W != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.W;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.W, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BH:
            if (blob_ptr->box.H != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.H;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.H, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
          case BD:
            if (blob_ptr->box.D != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.D;
              Serial.printf("\nBLOB:%d\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, ccPesets_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
        }
#endif
      }
    }
  }
}
