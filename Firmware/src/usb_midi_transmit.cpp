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

void e256_noteOn(byte channel, byte note, byte velocity){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);
  node_ptr->midiMsg.status = midi::NoteOn;
  node_ptr->midiMsg.data1 = note;
  node_ptr->midiMsg.data2 = velocity;
  llist_push_front(&midiIn, node_ptr);
};

void e256_noteOff(byte channel, byte note, byte velocity){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);
  node_ptr->midiMsg.status = midi::NoteOff;
  node_ptr->midiMsg.data1 = note;
  node_ptr->midiMsg.data2 = velocity;
  llist_push_front(&midiIn, node_ptr);
};

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

inline void printBytes(const uint8_t* data_ptr, uint16_t size) {
  Serial.printf("\nSysEx length: %d", size);
  Serial.printf("\nSysEx message: ");
  while (size > 0) {
    byte b = *data_ptr++;
    Serial.print(b, HEX);
    Serial.print(' ');
    size--;
  };
};

// Load config via MIDI system exclusive message
// [ SYSEX_BEGIN, SYSEX_ID, MODE, CONFIG_SIZE, SYSEX_END ] 
// ALLOC_DONE
// [ SYSEX_BEGIN, SYSEX_ID, MODE, IDENTIFIER, DATA, SYSEX_END ]
// LOAD_DONE

uint16_t midiBuffer = 290;
uint8_t* tmp_config_ptr = NULL;
uint16_t tmp_configSize = 0;

uint8_t* chunk_ptr = NULL;
uint8_t chunks = 0;
uint8_t chunkCount = 0;

uint16_t chunkSize = 0;
uint16_t lastChunkSize = 0;

void e256_systemExclusive(const uint8_t* data_ptr, uint16_t length, boolean complete){

  #if defined(DEBUG_CONFIG)
    printBytes(data_ptr, length);
  #endif

  uint8_t loadMode = *(data_ptr + 2); // [ ALLOCATE_MODE - LOAD_MODE ]

  if (loadMode == ALLOC_MODE){
    tmp_configSize = *(data_ptr + 3);
    tmp_config_ptr = (uint8_t*) malloc(tmp_configSize);
    chunk_ptr = tmp_config_ptr;
    chunks = (uint8_t)((tmp_configSize + 5) / midiBuffer);
    lastChunkSize = (tmp_configSize + 5) % midiBuffer;
    if (lastChunkSize != 0) chunks++;
    chunkCount = 0;
    usbMIDI.sendProgramChange(ALLOC_DONE, MIDI_OUTPUT_CHANNEL); // ProgramChange(program, channel);
    usbMIDI.send_now();
  }

  else if (loadMode == LOAD_MODE){

    uint8_t identifier = *(data_ptr + 3);
    
    if (identifier == SYSEX_CONF){
      if (chunks == 1) { // Only one chunk to load
        memcpy(tmp_config_ptr, data_ptr += 4, tmp_configSize);
      }
      else if (chunkCount == 0  && chunks > 1) { // First chunk
        chunkSize = midiBuffer - 4;
        memcpy(chunk_ptr, data_ptr += 4, chunkSize);
        chunk_ptr += midiBuffer;
        chunkCount++;
      }
      else if (chunkCount < chunks - 1){ // Middle chunks
        chunkSize = midiBuffer;
        memcpy(chunk_ptr, data_ptr, chunkSize);
        chunk_ptr += midiBuffer;
        chunkCount++;
      } else { // Last chunk
        chunkSize = lastChunkSize - 1;
        memcpy(chunk_ptr, data_ptr, chunkSize);
      };
    }
    else if (identifier == SYSEX_SOUND) {
      // TODO: copy to flash
      usbMIDI.sendProgramChange(LOAD_DONE, MIDI_OUTPUT_CHANNEL);
      usbMIDI.send_now();
    }
    else {
      usbMIDI.sendProgramChange(ERROR_UNKNOWN_SYSEX, MIDI_OUTPUT_CHANNEL);
      usbMIDI.send_now();
    };  
  };
};


void e256_clock(){
  Serial.println("Clock");
};

void USB_MIDI_TRANSMIT_SETUP(void) {
  usbMIDI.begin();
  usbMIDI.setHandleProgramChange(e256_programChange);
  usbMIDI.setHandleNoteOn(e256_noteOn);
  usbMIDI.setHandleNoteOff(e256_noteOff);
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