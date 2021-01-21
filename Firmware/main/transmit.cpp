/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "transmit.h"

#if SLIP_OSC
void SLIP_OSC_SETUP() {
  SLIPSerial.begin(BAUD_RATE); // FIXME
}

void blobs_usb_slipOsc(llist_t* blobs_ptr) {

  OSCBundle OSCbundle;
  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    OSCMessage msg("/b");
    msg.add(blob_ptr->UID);
    msg.add(blob_ptr->alive);
    msg.add(blob_ptr->centroid.X);
    msg.add(blob_ptr->centroid.Y);
    msg.add(blob_ptr->box.W);
    msg.add(blob_ptr->box.H);
    msg.add(blob_ptr->box.D);
    OSCbundle.add(msg);
  }
  SLIPSerial.beginPacket();     //
  OSCbundle.send(SLIPSerial);   // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();       // Mark the end of the OSC Packet
}
#endif

#if USB_MIDI
void USB_MIDI_SETUP() {
  usbMIDI.begin();
}

// Send blobs values using ControlChange MIDI format
// Send only the last blob that have been added to the sensor surface
// Separate blob's values according to the encoder position to allow the mapping into Max4Live
void blobs_usb_midi_learn(llist_t* blobs_ptr, preset_t* preset_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID == blobs_ptr->index) {
      switch (preset_ptr->val) {
        case 1:
          usbMIDI.sendControlChange(1, blob_ptr->alive, blobs_ptr->index + 1);
          break;
        case 2:
          usbMIDI.sendControlChange(2, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, 59.0, 0, 127)), blobs_ptr->index + 1);
          break;
        case 3:
          usbMIDI.sendControlChange(3, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, 59.0, 0, 127)), blobs_ptr->index + 1);
          break;
        case 4:
          usbMIDI.sendControlChange(4, blob_ptr->box.W, blobs_ptr->index + 1);
          break;
        case 5:
          usbMIDI.sendControlChange(5, blob_ptr->box.H, blobs_ptr->index + 1);
          break;
        case 6:
          usbMIDI.sendControlChange(6, constrain(blob_ptr->box.D, 0, 127), blobs_ptr->index + 1);
          break;
      }
    }
  }
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
}

// Send all blobs values using ControlChange MIDI format
void blobs_usb_midi_play(llist_t* blobs_ptr) {

  for (blob_t* blob_ptr = ITERATOR_START_FROM_HEAD(blobs_ptr); blob_ptr != NULL; blob_ptr = ITERATOR_NEXT(blob_ptr)) {
    usbMIDI.sendControlChange(1, blob_ptr->alive, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(2, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, 59.0, 0, 127)), blob_ptr->UID + 1);
    usbMIDI.sendControlChange(3, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, 59.0, 0, 127)), blob_ptr->UID + 1);
    usbMIDI.sendControlChange(4, blob_ptr->box.W, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(5, blob_ptr->box.H, blob_ptr->UID + 1);
    usbMIDI.sendControlChange(6, constrain(blob_ptr->box.D, 0, 127), blob_ptr->UID + 1);
  }
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
}

#endif
