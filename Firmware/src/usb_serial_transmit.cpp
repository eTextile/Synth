/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
  See: https://github.com/makeabilitylab/p5js/tree/master/WebSerial/Basic
  Demo: https://makeabilitylab.github.io/p5js/WebSerial/Basic/SliderOut/
*/

#include "usb_serial_transmit.h"

#include "config.h"
#include "llist.h"
#include "blob.h"

#define SERIAL_TRANSMIT_INTERVAL 10
uint32_t serialTransmitTimeStamp = 0;

void USB_SERIAL_TRANSMIT_SETUP(void) {
  //Serial.begin(BAUD_RATE);
};

void usb_serial_handle_input(void) {
  if (Serial.available() > 0) {
    // If we're here, then serial data has been received
    // Read data off the serial port until we get to the endline delimiter ('\n')
    // Store all of this data into a string
    String rcvdSerialData = Serial.readStringUntil('\n');
    int ledValue = rcvdSerialData.toInt();
    analogWrite(LED_BUILTIN, ledValue);
  };
};

void usb_serial_transmit(void) {
  switch (currentMode) {
    case RAW_MATRIX:
      /*
        if (millis() - oscTransmitTimeStamp > OSC_TRANSMIT_INTERVAL) {
        oscTransmitTimeStamp = millis();
        OSCMessage msg("/RAW");
        msg.add(rawFrame.pData, RAW_FRAME);
        SLIPSerial.beginPacket();
        msg.send(SLIPSerial);
        SLIPSerial.endPacket();
        };
      */
      break;
    case INTERP_MATRIX:
      /*
        if (millis() - oscTransmitTimeStamp > OSC_TRANSMIT_INTERVAL) {
        oscTransmitTimeStamp = millis();
        OSCMessage msg("/INTERP");
        msg.add(interpFrame.pData, NEW_FRAME);
        SLIPSerial.beginPacket();
        msg.send(SLIPSerial);
        SLIPSerial.endPacket();
        };
      */
      break;
    case BLOBS_PLAY:
    /*
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (blob_ptr->state) {
          if (!blob_ptr->lastState && blob_ptr->status == FREE) {
#if defined(DEBUG_SERIAL_TRANSMIT)
            Serial.printf("\nDEBUG_SERIAL_TRANSMIT: ON_%d", blob_ptr->UID);
#else
            OSCMessage msg("/ON");
            msg.add(blob_ptr->UID);
            SLIPSerial.beginPacket(); // Send SLIP header
            msg.send(SLIPSerial);     // Send the OSC bundle
            SLIPSerial.endPacket();   // Send the SLIP end of packet
#endif
          } else {
            if (millis() - blob_ptr->transmitTimeStamp > SERIAL_TRANSMIT_INTERVAL) {
              blob_ptr->transmitTimeStamp = millis();
#if defined(DEBUG_SERIAL_TRANSMIT)
              Serial.printf("\nDEBUG_SERIAL_TRANSMIT: UPDATE_%d", blob_ptr->UID);
#else
              OSCMessage msg("/UPDATE");
              msg.add(blob_ptr->UID);
              msg.add(blob_ptr->centroid.X);
              msg.add(blob_ptr->centroid.Y);
              msg.add(blob_ptr->centroid.Z);
              msg.add(blob_ptr->box.W);
              msg.add(blob_ptr->box.H);
              SLIPSerial.beginPacket(); // Send SLIP header
              msg.send(SLIPSerial);     // Send the OSC bundle
              SLIPSerial.endPacket();   // Send the SLIP end of packet
#endif
          };
        } else {
          if (blob_ptr->lastState && blob_ptr->status == TO_REMOVE) {
#if defined(DEBUG_SERIAL_TRANSMIT)
            Serial.printf("\nDEBUG_SERIAL_TRANSMIT: OFF_%d", blob_ptr->UID);
#else
            OSCMessage msg("/OFF");
            msg.add(blob_ptr->UID);
            SLIPSerial.beginPacket(); // Send SLIP header
            msg.send(SL      encoder.write(0x1);
      encoder.write(0x1);
IPSerial);     // Send the OSC bundle
            SLIPSerial.endPacket();   // Send the SLIP end of packet
#endif
          };
        };
      };
      */
      break;
    default:
      break;
  };
};