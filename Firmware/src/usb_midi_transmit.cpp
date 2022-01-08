/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "usb_midi_transmit.h"
#include "config.h"
#include "scan.h"
#include "llist.h"
#include "blob.h"
#include "midi_bus.h"
#include "allocate.h"

#define MIDI_TRANSMIT_INTERVAL 15
uint32_t usbTransmitTimeStamp = 0;

/*
void e256_noteOn(byte channel, byte note, byte velocity){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);
  node_ptr->midiMsg.status = midi::NoteOn;
  node_ptr->midiMsg.data1 = note;
  node_ptr->midiMsg.data2 = velocity;
  llist_push_front(&midiIn, node_ptr);
};
*/
/*
void e256_noteOff(byte channel, byte note, byte velocity){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);
  node_ptr->midiMsg.status = midi::NoteOff;
  node_ptr->midiMsg.data1 = note;
  node_ptr->midiMsg.data2 = velocity;
  llist_push_front(&midiIn, node_ptr);
};
*/

void e256_controlChange(byte channel, byte control, byte value){
  set_level(control, value);
};

void e256_programChange(byte channel, byte program){
  set_mode(program);
};

void error(uint8_t err){
  usbMIDI.sendProgramChange(err, MIDI_OUTPUT_CHANNEL); // ProgramChange(program, channel);
  usbMIDI.send_now();
  #if defined(DEBUG_CONFIG)
    Serial.printf("\nCONFIG_ERROR\t%d", err);
  #endif
};

inline void printBytes(const byte* data_ptr, unsigned int size) {
  while (size > 0) {
    byte b = *data_ptr++;
    if (b < 16) Serial.print('0');
    Serial.print(b, HEX);
    if (size > 1) Serial.print(' ');
    size = size - 1;
  };
};

// Load config via MIDI system exclusive message
void e256_systemExclusive(uint8_t* data_ptr, unsigned int length){
  #if defined(DEBUG_CONFIG)
    Serial.print("SysEx Message: ");
    printBytes(data_ptr, length);
    Serial.println();
  #endif
  uint8_t* identifier = data_ptr + 2 * sizeof(uint8_t);
  data_ptr += 3 * sizeof(uint8_t);

  if (*identifier == MAPPING_CONFIG){
    configSize = length - 3;

    config_ptr = allocate((char*)data_ptr, configSize);
    strcpy(config_ptr, (char*)data_ptr);
    load_config(config_ptr);
    set_mode(MAPPING_LIB);
  }
  else if (*identifier == AUDIO_FILE) {
    // TODO
  }
  else {
    // Unknown system exclusive message
  }
};

void e256_clock(){
  Serial.println("Clock");
};

void USB_MIDI_TRANSMIT_SETUP(void) {
  usbMIDI.begin();
  usbMIDI.setHandleProgramChange(e256_programChange);
  //usbMIDI.setHandleNoteOn(e256_noteOn);
  //usbMIDI.setHandleNoteOff(e256_noteOff);
  usbMIDI.setHandleControlChange(e256_controlChange);
  usbMIDI.setHandleSystemExclusive(e256_systemExclusive);
  usbMIDI.setHandleClock(e256_clock);
};

void usb_midi_read_input(void) {
  usbMIDI.read(MIDI_INPUT_CHANNEL);         // Is there a MIDI incoming messages on channel One
  while (usbMIDI.read(MIDI_INPUT_CHANNEL)); // Read and discard any incoming MIDI messages
};

void usb_midi_transmit(void) {
  switch (playMode) {
    case RAW_MATRIX:
      if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
        usbTransmitTimeStamp = millis();
        usbMIDI.sendSysEx(RAW_FRAME, rawFrame.pData);
        usbMIDI.send_now();
        while (usbMIDI.read()); // Read and discard any incoming MIDI messages
      };
      break;
    case INTERP_MATRIX:
      if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
        usbTransmitTimeStamp = millis();
        // NOT_WORKING: see https://forum.pjrc.com/threads/28282-How-big-is-the-MIDI-receive-buffer
        //usbMIDI.sendSysEx(NEW_FRAME, interpFrame.pData, false, 0);
        //usbMIDI.send_now();
        //while (usbMIDI.read()); // Read and discard any incoming MIDI messages
      };
      break;
    case BLOBS_PLAY:
      // Send all blobs values over USB using MIDI format
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (blob_ptr->state) {
          if (!blob_ptr->lastState) {
            usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, BS); // sendNoteOn(note, velocity, channel);
            usbMIDI.send_now();
          } else {
            if (millis() - blob_ptr->transmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
              blob_ptr->transmitTimeStamp = millis();
              // usbMIDI.sendControlChange(control, value, channel);
              usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BZ, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
              usbMIDI.send_now();
            };
          };
        } else {
          if (blob_ptr->lastState && blob_ptr->status != NOT_FOUND) {
            usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, BS); // sendNoteOff(note, velocity, channel);
            usbMIDI.send_now();
          };
        };
        while (usbMIDI.read()); // Read and discard any incoming MIDI messages
      };
      break;
    case BLOBS_LEARN:
        /*
        // Send separate blobs values using Control Change MIDI format
        // Send only the last blob that have been added to the sensor surface
        // Select blob's values according to the encoder position to allow the auto-mapping into Max4Live...
        if ((blob_t*)llist_blobs.tail_ptr != NULL) {
          blob_t* blob_ptr = (blob_t*)llist_blobs.tail_ptr;
          switch (e256_ctr.levels[BLOBS_LEARN].val) {
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
              usbMIDI.sendControlChange(blob_ptr->UID + 1, (uint8_t)round(map(blob_ptr->centroid.X, 0, X_MAX - X_MIN , 0, 127)), BX);
              break;
            case BY:
              usbMIDI.sendControlChange(blob_ptr->UID + 1, (uint8_t)round(map(blob_ptr->centroid.Y, 0, X_MAX - X_MIN, 0, 127)), BY);
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
      */
      break;
    case MAPPING_LIB:
      for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
        switch (node_ptr->midiMsg.status) {
          case midi::NoteOn:
            usbMIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);  // USB send MIDI noteOn
            break;
          case midi::NoteOff:
            usbMIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL);  // USB send MIDI noteOff
            break;
          case midi::ControlChange:
            if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
              usbTransmitTimeStamp = millis();
              usbMIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_OUTPUT_CHANNEL); // USB send MIDI control_change
            };
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