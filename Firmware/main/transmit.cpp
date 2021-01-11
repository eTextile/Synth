/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "transmit.h"

/*
  void TRANSMIT_SLIP_OSC_SETUP() {
  SLIPSerial.begin(BAUD_RATE);
  }
*/

void TRANSMIT_MIDI_SETUP() {
  usbMIDI.begin();
}

/*
  void transmit_blobs_slipOsc(llist_t* blobs_ptr, uint8_t* valSelector_ptr) {

  OSCBundle OSCbundle;

  for (blob_t* blob = ITERATOR_START_FROM_HEAD(blobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {

    blobPacket[0] = blob->UID;        // uint8_t unique session ID
    blobPacket[1] = blob->alive;      // uint8_t
    blobPacket[2] = blob->centroid.X; // uint8_t
    blobPacket[3] = blob->centroid.Y; // uint8_t
    blobPacket[4] = blob->box.W;      // uint8_t
    blobPacket[5] = blob->box.H;      // uint8_t
    blobPacket[6] = (blob->box.D - threshold); // uint8_t

    OSCMessage msg("/b");
    msg.add(blobPacket, BLOB_PACKET_SIZE);
    OSCbundle.add(msg);
  }
  SLIPSerial.beginPacket();     //
  OSCbundle.send(SLIPSerial);   // Send the bytes to the SLIP stream
  SLIPSerial.endPacket();       // Mark the end of the OSC Packet
  }
*/

// Send all blobs in MIDI format (AfterTouchPoly)
// usbMIDI.sendControlChange(control, value, channel);
void transmit_blobs_midi(llist_t* blobs_ptr, preset_t* preset_ptr) {

  int pos = 0;
  for (blob_t* blob = ITERATOR_START_FROM_HEAD(blobs_ptr); blob != NULL; blob = ITERATOR_NEXT(blob)) {

    pos = blob->UID * 10;

    switch (preset_ptr->val) {
      case 0:
        usbMIDI.sendAfterTouchPoly(0 + pos, blob->alive, 1);
        usbMIDI.sendAfterTouchPoly(1 + pos, blob->centroid.X, 1);
        usbMIDI.sendAfterTouchPoly(2 + pos, blob->centroid.Y, 1);
        usbMIDI.sendAfterTouchPoly(3 + pos, blob->box.W, 1);
        usbMIDI.sendAfterTouchPoly(4 + pos, blob->box.H, 1);
        usbMIDI.sendAfterTouchPoly(5 + pos, blob->box.D >> 1, 1);
      case 1:
        usbMIDI.sendAfterTouchPoly(0 + pos, blob->alive, 1);
        break;
      case 2:
        usbMIDI.sendAfterTouchPoly(1 + pos, blob->centroid.X, 1);
        break;
      case 3:
        usbMIDI.sendAfterTouchPoly(2 + pos, blob->centroid.Y, 1);
        break;
      case 4:
        usbMIDI.sendAfterTouchPoly(3 + pos, blob->box.W, 1);
        break;
      case 5:
        usbMIDI.sendAfterTouchPoly(4 + pos, blob->box.H, 1);
        break;
      case 6:
        usbMIDI.sendAfterTouchPoly(5 + pos, blob->box.D >> 1, 1);
        break;
    }
  }
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
}
