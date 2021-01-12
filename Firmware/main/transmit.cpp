/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "transmit.h"

#if USB_SLIP_OSC
void USB_SLIP_OSC_SETUP() {
  SLIPSerial.begin(BAUD_RATE); // FIXME
}

void blobs_usb_slipOsc(blob_t* blob_ptr, preset_t* preset_ptr, OSCBundle* synthOSC) {

  OSCMessage msg("/b");
  msg.add((uint8_t)blob_ptr->UID);                        // uint8_t unique session ID
  msg.add((uint8_t)blob_ptr->alive);                      // uint8_t
  msg.add((float)blob_ptr->centroid.X);                   // float
  msg.add((float)blob_ptr->centroid.Y);                   // float
  msg.add((uint8_t)blob_ptr->box.W);                      // uint8_t
  msg.add((uint8_t)blob_ptr->box.H);                      // uint8_t
  msg.add((uint8_t)(blob_ptr->box.D - preset_ptr->val));  // uint8_t
  synthOSC->add(msg);
}
#endif

#if USB_MIDI
void USB_MIDI_SETUP() {
  usbMIDI.begin();
}

// Send blobs in ControlChange MIDI format
// Separate blob's values to allow the mapping into Max4Live
void blob_usb_midi_learn(llist_t* blobs_ptr, preset_t* preset_ptr) {

  for (blob_t* blob = ITERATOR_START_FROM_HEAD(blobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {

    switch (preset_ptr->val) {
      case 1:
        usbMIDI.sendControlChange(1, blob->alive, blobs_ptr->index + 1);
        break;
      case 2:
        usbMIDI.sendControlChange(2, (uint8_t)round(blob->centroid.X), blobs_ptr->index + 1);
        break;
      case 3:
        usbMIDI.sendControlChange(3, (uint8_t)round(blob->centroid.Y), blobs_ptr->index + 1);
        break;
      case 4:
        usbMIDI.sendControlChange(4, blob->box.W, blobs_ptr->index + 1);
        break;
      case 5:
        usbMIDI.sendControlChange(5, blob->box.H, blobs_ptr->index + 1);
        break;
      case 6:
        usbMIDI.sendControlChange(6, blob->box.D >> 1, blobs_ptr->index + 1);
        break;
    }
  }
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
}

void blob_usb_midi_play(llist_t* blobs_ptr) {

  for (blob_t* blob = ITERATOR_START_FROM_HEAD(blobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {
    usbMIDI.sendControlChange(0, blob->alive, blob->UID + 1);
    usbMIDI.sendControlChange(1, (uint8_t)round(blob->centroid.X), blob->UID + 1);
    usbMIDI.sendControlChange(2, (uint8_t)round(blob->centroid.Y), blob->UID + 1);
    usbMIDI.sendControlChange(3, blob->box.W, blob->UID + 1);
    usbMIDI.sendControlChange(4, blob->box.H, blob->UID + 1);
    usbMIDI.sendControlChange(5, blob->box.D >> 1, blob->UID + 1);
  }
}

#endif
