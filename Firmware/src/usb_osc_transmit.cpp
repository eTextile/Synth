/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
  TODO: Lufa_lib...
*/

#include "usb_osc_transmit.h"

#if defined(USB_OSC)

SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);

#define OSC_TRANSMIT_INTERVAL 5
unsigned long oscTransmitTimeStamp = 0;

void USB_OSC_TRANSMIT_SETUP(void) {
  SLIPSerial.begin(BAUD_RATE);
};

void usb_osc_read_input(void) {
  OSCMessage bundleIN;
  int size;
  if (SLIPSerial.available()) {
    while (!SLIPSerial.endofPacket()) {
      if (size = SLIPSerial.available()) {
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

// INPUT_CONTROL
void usb_osc_handle_input(OSCMessage & msg) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  switch (msg.getInt(0)) {
    case midi::NoteOn:
      node_ptr->midiMsg.status = midi::NoteOn;         // Set the MIDI status
      node_ptr->midiMsg.data1 = msg.getInt(1);         // Set the MIDI note
      node_ptr->midiMsg.data2 = msg.getInt(2);         // Set the MIDI velocity
      //node_ptr->midiMsg.channel = msg.getInt(3);     // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    case midi::NoteOff:
      node_ptr->midiMsg.status = midi::NoteOff;        // Set the MIDI status
      node_ptr->midiMsg.data1 = msg.getInt(1);         // Set the MIDI note
      node_ptr->midiMsg.data2 = msg.getInt(2);         // Set the MIDI velocity
      //node_ptr->midiMsg.channel = msg.getInt(3);     // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    case midi::ControlChange:
      node_ptr->midiMsg.status = midi::ControlChange;  // Set the MIDI status
      node_ptr->midiMsg.data1 = msg.getInt(1);         // Set the MIDI note
      node_ptr->midiMsg.data2 = msg.getInt(2);         // Set the MIDI velocity
      //node_ptr->midiMsg.channel = msg.getInt(3);     // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    case midi::Clock:
      node_ptr->midiMsg.status = midi::Clock;          // Set the MIDI status
      node_ptr->midiMsg.data1 = msg.getInt(1);         // Set the MIDI note
      node_ptr->midiMsg.data2 = msg.getInt(2);         // Set the MIDI velocity
      //node_ptr->midiMsg.channel = msg.getInt(3);       // Set the MIDI channel
      llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
      break;
    default:
      llist_push_front(&midi_node_stack, node_ptr);    // Add the node to the midi_node_stack linked liste
      break;
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
          if (!blob_ptr->lastState && blob_ptr->status == FREE) {
#if defined(DEBUG_OSC_TRANSMIT)
            Serial.printf("\nDEBUG_OSC_TRANSMIT: ON_%d", blob_ptr->UID);
#else
            OSCMessage msg("/ON");
            msg.add(blob_ptr->UID);
            SLIPSerial.beginPacket(); // Send SLIP header
            msg.send(SLIPSerial);     // Send the OSC bundle
            SLIPSerial.endPacket();   // Send the SLIP end of packet
#endif
          } else {
            if (millis() - blob_ptr->transmitTimeStamp > OSC_TRANSMIT_INTERVAL) {
              blob_ptr->transmitTimeStamp = millis();
#if defined(DEBUG_OSC_TRANSMIT)
              Serial.printf("\nDEBUG_OSC_TRANSMIT: UPDATE_%d", blob_ptr->UID);
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
          };
        } else {
          if (blob_ptr->lastState && blob_ptr->status == TO_REMOVE) {
#if defined(DEBUG_OSC_TRANSMIT)
            Serial.printf("\nDEBUG_OSC_TRANSMIT: OFF_%d", blob_ptr->UID);
#else
            OSCMessage msg("/OFF");
            msg.add(blob_ptr->UID);
            SLIPSerial.beginPacket(); // Send SLIP header
            msg.send(SLIPSerial);     // Send the OSC bundle
            SLIPSerial.endPacket();   // Send the SLIP end of packet
#endif
          };
        };
      };
      break;
    default:
      break;
  };
};
#endif