/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "usb_slip_osc_transmit.h"

#if USB_SLIP_OSC_TRANSMIT

SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);

void USB_SLIP_OSC_SETUP(void) {
  SLIPSerial.begin(BAUD_RATE);
}

void usb_slip_osc_handle_input(void) {

  OSCMessage request;

  int size;

  while (!SLIPSerial.endofPacket()) {
    if ((size = SLIPSerial.available()) > 0) {
      while (size--)
        request.fill(SLIPSerial.read());
    }
  }

  if (!request.hasError()) {
    if (request.fullMatch("/c")) { // Calibrate
      lastMode = currentMode;
      currentMode = CALIBRATE;
      presets[CALIBRATE].setLed = true;
      presets[CALIBRATE].updateLed = true;
    }
    else if (request.fullMatch("/t")) { // Set threshold
      /*
        lastMode = currentMode;
        currentMode = THRESHOLD;
        presets[THRESHOLD].val = map(value, 0, 127, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
        encoder.write(presets[THRESHOLD].val << 2);
        interpThreshold = constrain(presets[THRESHOLD].val - 5, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
        presets[THRESHOLD].setLed = true;
        presets[THRESHOLD].ledVal = map(value, 0, 127, 0, 255);
        presets[THRESHOLD].updateLed = true;
        presets[THRESHOLD].update = true;
      */
    }
    else if (request.fullMatch("/r")) { // Get raw datas
      OSCMessage m("/r");
      m.add(rawFrame.pData, RAW_FRAME);
      SLIPSerial.beginPacket();
      m.send(SLIPSerial);
      SLIPSerial.endPacket();
    }
    else if (request.fullMatch("/i")) { // Get interp
      OSCMessage m("/i");
      m.add(interpFrame.pData, NEW_FRAME);
      SLIPSerial.beginPacket();
      m.send(SLIPSerial);
      SLIPSerial.endPacket();
    }
    else if (request.fullMatch("/b")) { // Get blobs
      OSCBundle OSCbundle;
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        OSCMessage msg("/b");
        //msg.add((uint8_t*)&llist_blobs, 14); // FIXME
        msg.add(blob_ptr->UID);
        msg.add(blob_ptr->state);
        msg.add(blob_ptr->lastState);
        msg.add(blob_ptr->centroid.X);
        msg.add(blob_ptr->centroid.Y);
        msg.add(blob_ptr->box.W);
        msg.add(blob_ptr->box.H);
        msg.add(blob_ptr->centroid.Z);
        OSCbundle.add(msg);
      }
      SLIPSerial.beginPacket();     // Send SLIP header
      OSCbundle.send(SLIPSerial);   // Send the OSC bundle
      SLIPSerial.endPacket();       // Send the SLIP end of packet
    }
  }
}
#endif
