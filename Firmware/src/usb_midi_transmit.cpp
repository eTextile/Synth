/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "usb_midi_transmit.h"

#include "config.h"
#include "llist.h"
#include "blob.h"
#include "midi_bus.h"

#if defined(USB_MIDI) || defined(USB_MIDI_SERIAL) || defined(USB_MTPDISK_MIDI)

#define MIDI_TRANSMIT_INTERVAL 10
unsigned long int usbTransmitTimeStamp = 0;

void usb_midi_handle_cc(byte channel, byte control, byte value){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.status = midi::ControlChange;  // Set the MIDI status
  node_ptr->midiMsg.data1 = control;               // Set the MIDI control
  node_ptr->midiMsg.data2 = value;                 // Set the MIDI value
  //node_ptr->midiMsg.channel = channel;           // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);             // Add the node to the midiIn linked liste
};

void USB_MIDI_TRANSMIT_SETUP(void) {
  usbMIDI.begin();
  usbMIDI.setHandleControlChange(usb_midi_handle_cc);
};

void usb_midi_read_input(void) {
  usbMIDI.read(MIDI_INPUT_CHANNEL);         // Is there a MIDI incoming messages on channel One
  while (usbMIDI.read(MIDI_INPUT_CHANNEL)); // Read and discard any incoming MIDI messages
};

void usb_midi_transmit(void) {
  switch (currentMode) {
    case RAW_MATRIX:
      if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
        usbTransmitTimeStamp = millis();
        usbMIDI.sendSysEx(RAW_FRAME, rawFrame.pData, false, 0);
        usbMIDI.send_now();
      };
      break;
    case INTERP_MATRIX:
      if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
        usbTransmitTimeStamp = millis();
        // NOT_WORKING > You can use OSC insted of MIDI!
        // See https://forum.pjrc.com/threads/28282-How-big-is-the-MIDI-receive-buffer
        //usbMIDI.sendSysEx(NEW_FRAME, interpFrame.pData, false, 0);
        //usbMIDI.send_now();
      };
      break;
    case BLOBS_PLAY:
      // Send all blobs values over USB using MIDI format
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (blob_ptr->state) {
          if (!blob_ptr->lastState) {
            usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, BS); // sendNoteOn(note, velocity, channel);
            usbMIDI.send_now();
            //while (usbMIDI.read()); // Read and discard any incoming MIDI messages
          } else {
            if (millis() - blob_ptr->transmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
              blob_ptr->transmitTimeStamp = millis();
              // usbMIDI.sendControlChange(control, value, channel);
              usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BZ, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
            };
          };
        } else {
          if (blob_ptr->lastState && blob_ptr->status != NOT_FOUND) {
            usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, BS); // sendNoteOff(note, velocity, channel);
            usbMIDI.send_now();
            //while (usbMIDI.read()); // Read and discard any incoming MIDI messages
          };
        };
        //usbMIDI.send_now();
        while (usbMIDI.read()); // Read and discard any incoming MIDI messages
      };
      break;
    case BLOBS_LEARN:
        // Send separate blobs values using Control Change MIDI format
        // Send only the last blob that have been added to the sensor surface
        // Select blob's values according to the encoder position to allow the auto-mapping into Max4Live...
        if ((blob_t*)llist_blobs.tail_ptr != NULL) {
          blob_t* blob_ptr = (blob_t*)llist_blobs.tail_ptr;
          switch (presets[BLOBS_LEARN].val) {
            case BS:
              if (blob_ptr->state) {
                if (!blob_ptr->lastState) {
                  usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, BS); // sendNoteOn(note, velocity, channel);
                };
              }
              else {
                usbMIDI.sendNoteOn(blob_ptr->UID + 1, 0, BS); // sendNoteOn(note, velocity, channel);
              };
              break;
            case BX:
              usbMIDI.sendControlChange(blob_ptr->UID + 1, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, X_MAX - X_MIN , 0, 127)), BX);
              break;
            case BY:
              usbMIDI.sendControlChange(blob_ptr->UID + 1, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, X_MAX - X_MIN, 0, 127)), BY);
              break;
            case BZ:
              usbMIDI.sendControlChange(blob_ptr->UID + 1, constrain(blob_ptr->centroid.Z, 0, 127), BZ);
              break;
            case BW:
              usbMIDI.sendControlChange(blob_ptr->UID + 1, blob_ptr->box.W, BW);
              break;
            case BH:
              usbMIDI.sendControlChange(blob_ptr->UID + 1, blob_ptr->box.H, BH);
              break;
            default:
              break;
          };
        };
      break;
    case MAPPING_LIB:
      for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
        switch (node_ptr->midiMsg.status) {
          case MAPPING_LIB:
          case midi::NoteOn:
            usbMIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);  // USB send MIDI noteOn
            break;
          case midi::NoteOff:
            usbMIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);  // USB send MIDI noteOff
            break;
          case midi::ControlChange:
            usbMIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL); // USB send MIDI control_change
            break;
          default:
            break;
        };
      };
      break;
    default:
      break;
  };
};
#endif