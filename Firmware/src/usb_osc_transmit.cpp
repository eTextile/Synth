/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
  TODO: Lufa_lib...
*/

#include "usb_osc_transmit.h"

#include "llist.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"

#include <OSCBoards.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <SLIPEncodedSerial.h>

SLIPEncodedSerial SLIPSerial(Serial1);

#define OSC_TRANSMIT_INTERVAL 5
unsigned long oscTransmitTimeStamp = 0;

void USB_OSC_TRANSMIT_SETUP(void) {
  SLIPSerial.begin(BAUD_RATE);
};

void usb_osc_handle_input(OSCMessage &msg, int addrOffset) {
  osc_getInput(msg.getInt(0), msg.getInt(1), msg.getInt(2));
};

void usb_osc_read_input(void) {
  OSCMessage bundleIN;
  int size;
  if (SLIPSerial.available()) {
    while (!SLIPSerial.endofPacket()) {
      if (SLIPSerial.available()) {
        size = SLIPSerial.available();
        while (size--)
          bundleIN.fill(SLIPSerial.read());
      };
    };
  };
  if (!bundleIN.hasError()) {
    //if (bundleIN.fullMatch("/C")) { // Calibrate
      //bundleIN.dispatch("/C", handle_osc_input);
      bundleIN.route("/C", usb_osc_handle_input);
    //};
  };
};

void usb_osc_transmit(void) {
  switch (currentMode) {
    case RAW_MATRIX:
      if (millis() - oscTransmitTimeStamp > OSC_TRANSMIT_INTERVAL) {
        oscTransmitTimeStamp = millis();
        OSCMessage msg("/RAW");
        msg.add(rawFrame.pData, RAW_FRAME);
        SLIPSerial.beginPacket();
        msg.send(SLIPSerial);
        SLIPSerial.endPacket();
      };
      break;
    case INTERP_MATRIX:
      if (millis() - oscTransmitTimeStamp > OSC_TRANSMIT_INTERVAL) {
        oscTransmitTimeStamp = millis();
        OSCMessage msg("/INTERP");
        msg.add(interpFrame.pData, NEW_FRAME);
        SLIPSerial.beginPacket();
        msg.send(SLIPSerial);
        SLIPSerial.endPacket();
      };
      break;
    case BLOBS_PLAY:
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (blob_ptr->state) {
          //if (!blob_ptr->lastState && blob_ptr->status == FREE) {
          if (!blob_ptr->lastState) {
            #if defined(DEBUG_OSC_TRANSMIT)
            Serial.printf("\nDEBUG_OSC_TRANSMIT\tON:%d", blob_ptr->UID);
            #else
            OSCMessage msg("/ON");
            msg.add(blob_ptr->UID);
            SLIPSerial.beginPacket();
            msg.send(SLIPSerial);
            SLIPSerial.endPacket();
            #endif
          } else {
            if (millis() - blob_ptr->transmitTimeStamp > OSC_TRANSMIT_INTERVAL) {
              blob_ptr->transmitTimeStamp = millis();
              #if defined(DEBUG_OSC_TRANSMIT)
              Serial.printf("\nDEBUG_OSC_TRANSMIT\tUPDATE:%d", blob_ptr->UID);
              #else
              OSCMessage msg("/UPDATE");
              msg.add(blob_ptr->UID);
              msg.add(blob_ptr->centroid.X);
              msg.add(blob_ptr->centroid.Y);
              msg.add(blob_ptr->centroid.Z);
              msg.add(blob_ptr->box.W);
              msg.add(blob_ptr->box.H);
              SLIPSerial.beginPacket();
              msg.send(SLIPSerial);
              SLIPSerial.endPacket();
              #endif
            };
          };
        } else {
          //if (blob_ptr->lastState && blob_ptr->status == TO_REMOVE) {
            #if defined(DEBUG_OSC_TRANSMIT)
            Serial.printf("\nDEBUG_OSC_TRANSMIT\tOFF:%d", blob_ptr->UID);
            #else
            OSCMessage msg("/OFF");
            msg.add(blob_ptr->UID);
            SLIPSerial.beginPacket();
            msg.send(SLIPSerial);
            SLIPSerial.endPacket();
            #endif
          //};
        };
      };
      break;
      case BLOBS_LEARN:
      //TODO
      break;
    default:
      break;
  };
};