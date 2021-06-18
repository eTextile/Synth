/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.

  What about the TUIO 1.1 Protocol Specification
  http://www.tuio.org/?specification
  What about the MIDI_MPE Protocol Specification
  https://www.midi.org/midi-articles/midi-polyphonic-expression-mpe
*/

#include "usb_midi_transmit.h"

#if USB_MIDI_TRANSMIT

midiNode_t midiInArray[MAX_SYNTH] = {0}; // 1D Array to alocate memory for incoming midi notes
llist_t  midiNodes_stack;                // Midi nodes stack

void usb_midi_llist_init(llist_t* nodes_ptr, midiNode_t* nodeArray_ptr, const int nodes) {
  llist_raz(nodes_ptr);
  for (int i = 0; i < nodes; i++) {
    llist_push_front(nodes_ptr, &nodeArray_ptr[i]);
  }
}

void USB_MIDI_SETUP(void) {
  usb_midi_llist_init(&midiNodes_stack, &midiInArray[0], MAX_SYNTH);
  usbMIDI.begin();
  usbMIDI.setHandleControlChange(usb_midi_update_presets);
};

void usb_midi_handle_input(void) {
  usbMIDI.read();
  if (currentMode == MIDI_LEARN) {
    usb_midi_learn();
  }
  else {
    usb_midi_send_blobs();
  };
};

void usb_midi_update_presets(byte channel, byte control, byte value) {
  switch (control) {
    case CALIBRATE: // CONTROL 5
      lastMode = currentMode;
      currentMode = CALIBRATE;
      presets[CALIBRATE].setLed = true;
      presets[CALIBRATE].updateLed = true;
      break;
    case THRESHOLD: // CONTROL 3
      lastMode = currentMode;
      currentMode = THRESHOLD;
      presets[THRESHOLD].val = map(value, 0, 127, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
      encoder.write(presets[THRESHOLD].val << 2);
      interpThreshold = constrain(presets[THRESHOLD].val - 5, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
      presets[THRESHOLD].setLed = true;
      presets[THRESHOLD].ledVal = map(value, 0, 127, 0, 255);
      presets[THRESHOLD].updateLed = true;
      presets[THRESHOLD].update = true;
      break;
    default:
      break;
  };
};

void usb_midi_send_raw(void) {
  usbMIDI.sendSysEx(RAW_FRAME, &rawFrame.pData[0]);
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
}

void usb_midi_send_interp(void) {
  usbMIDI.sendSysEx(NEW_FRAME, &interpFrame.pData[0]);
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
}

// Send blobs values using ControlChange MIDI format
// Send only the last blob that have been added to the sensor surface
// Separate blob's values according to the encoder position to allow the mapping into Max4Live
void usb_midi_learn(void) {
  blob_t* blob_ptr = (blob_t*)blobs.tail_ptr;
  if (blob_ptr != NULL) {
    switch (presets[MIDI_LEARN].val) {
      case BS:
        //usbMIDI.sendControlChange(BS, blob_ptr->state, blob_ptr->UID + 1);
        if (!blob_ptr->lastState) usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, 0);
        if (!blob_ptr->state) usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, 0);
        break;
      case BL:
        usbMIDI.sendControlChange(BS, blob_ptr->lastState, blob_ptr->UID + 1);
        break;
      case BX:
        usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, 59.0, 0, 127)), blob_ptr->UID + 1);
        break;
      case BY:
        usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, 59.0, 0, 127)), blob_ptr->UID + 1);
        break;
      case BW:
        usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
        break;
      case BH:
        usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
        break;
      case BD:
        usbMIDI.sendControlChange(BD, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
        break;
    }
    while (usbMIDI.read()); // Read and discard any incoming MIDI messages
  }
}

// Send all blobs values using MIDI format
void usb_midi_send_blobs(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (!blob_ptr->lastState) usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, 0);
    usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, X_MAX, 0, 127)), blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, Y_MAX, 0, 127)), blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(BD, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
    /*
      usbMIDI.sendAfterTouchPoly(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, X_MAX, 0, 127)), blob_ptr->UID + 1);
      usbMIDI.sendAfterTouchPoly(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, Y_MAX, 0, 127)), blob_ptr->UID + 1);
      usbMIDI.sendAfterTouchPoly(BW, blob_ptr->box.W, blob_ptr->UID + 1);
      usbMIDI.sendAfterTouchPoly(BH, blob_ptr->box.H, blob_ptr->UID + 1);
      usbMIDI.sendAfterTouchPoly(BD, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
    */
    if (!blob_ptr->state) usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, 0);
  }
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
}

// ccPesets_ptr -> ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]
void controlChange(llist_t* llist_ptr, ccPesets_t* ccPesets_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(llist_ptr); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    // Test if we are within the blob limit
    if (blob_ptr->UID == ccPesets_ptr->blobID) {
      // Test if the blob is alive
      if (blob_ptr->state) {
        switch (ccPesets_ptr->mappVal) {
          case BX:
            if (blob_ptr->centroid.X != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.X;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, round(blob_ptr->centroid.X), ccPesets_ptr->midiChannel);
            }
            break;
          case BY:
            if (blob_ptr->centroid.Y != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Y;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, round(blob_ptr->centroid.Y), ccPesets_ptr->midiChannel);
            }
            break;
          case BW:
            if (blob_ptr->box.W != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.W;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, blob_ptr->box.W, ccPesets_ptr->midiChannel);
            }
            break;
          case BH:
            if (blob_ptr->box.H != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.H;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, blob_ptr->box.H, ccPesets_ptr->midiChannel);
            }
            break;
          case BD:
            if (blob_ptr->centroid.Z != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Z;
              usbMIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.Z, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
        }
#if DEBUG_MIDI_USB
        switch (ccPesets_ptr->mappVal) {
          case BX:
            if (blob_ptr->centroid.X != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.X;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, blob_ptr->centroid.X, ccPesets_ptr->midiChannel);
            }
            break;
          case BY:
            if (blob_ptr->centroid.Y != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Y;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, blob_ptr->centroid.Y, ccPesets_ptr->midiChannel);
            }
            break;
          case BW:
            if (blob_ptr->box.W != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.W;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, blob_ptr->box.W, ccPesets_ptr->midiChannel);
            }
            break;
          case BH:
            if (blob_ptr->box.H != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.H;
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, blob_ptr->box.H, ccPesets_ptr->midiChannel);
            }
            break;
          case BD:
            if (blob_ptr->centroid.Z != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Z;
              Serial.printf("\nBLOB:%d\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, ccPesets_ptr->cChange, constrain(blob_ptr->centroid.Z, 0, 127), ccPesets_ptr->midiChannel);
            }
            break;
        }
#endif
      }
    }
  }
}
#endif
