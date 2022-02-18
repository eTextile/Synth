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
  //Serial.printf("\ne256_programChange: %d_%d", channel, program);
  // BUG with usbMIDI.setHandleProgramChange()
  switch (channel){
  case 1: // CHANNEL 1 -> MODE
    switch (program){
    case RAW_MATRIX:
      set_mode(RAW_MATRIX);
      break;
    case BLOBS_PLAY:
      set_mode(BLOBS_PLAY);
      break;
    case MAPPING_LIB:
      set_mode(MAPPING_LIB);
      break;
    };
  case 2: // CHANNEL 2 -> STATE
    switch (program){
    case CALIBRATE:
      matrix_calibrate();
      break;
    case DONE_ACTION:
      set_state(DONE_ACTION);
      break;
    case ERROR:
      set_state(ERROR);
      break;
    };
  };
};

void midiInfo(uint8_t msg){
  usbMIDI.sendProgramChange(msg, MIDI_OUTPUT_CHANNEL); // ProgramChange(program, channel);
  usbMIDI.send_now();
  #if defined(DEBUG_CONFIG)
    Serial.printf("\nMIDI_MSG:\t%d", msg);
  #endif
};

inline void printBytes(const uint8_t* data_ptr, uint16_t size) {
  Serial.printf("\nSysEx length: %d", size);
  Serial.printf("\nSysEx message:");
  while (size > 0) {
    byte b = *data_ptr++;
    Serial.printf(" %x", b);
    size--;
  };
};

// Load config via MIDI system exclusive message
// [ SYSEX_BEGIN, SYSEX_ID, SYSEX_IDENTIFIER, SYSEX_SIZE_MSB, SYSEX_SIZE_LSB, SYSEX_END ]
// ALLOC_DONE
// [ SYSEX_BEGIN, SYSEX_ID, SYSEX_DATA, SYSEX_END ]
// LOAD_DONE

boolean sysEx_alloc = true;

uint8_t sysEx_identifier = 0;
uint16_t sysEx_dataSize = 0;
uint8_t* sysEx_data_ptr = NULL;

uint8_t sysEx_chunks = 0;
uint8_t sysEx_chunkCount = 0;
uint16_t sysEx_chunkSize = 0;
uint16_t sysEx_lastChunkSize = 0;
uint8_t* sysEx_chunk_ptr = NULL;

void e256_systemExclusive(const uint8_t* data_ptr, uint16_t length, boolean complete){
  if (sysEx_alloc){
    sysEx_alloc = false;
    sysEx_identifier = *(data_ptr + 2);
    sysEx_dataSize = *(data_ptr + 3) << 7 | *(data_ptr + 4);    
    sysEx_chunk_ptr = sysEx_data_ptr = allocate(sysEx_data_ptr, sysEx_dataSize);
    sysEx_chunks = (uint8_t)((sysEx_dataSize + 3) / USB_MIDI_SYSEX_MAX);
    sysEx_lastChunkSize = (sysEx_dataSize + 3) % USB_MIDI_SYSEX_MAX;
    if (sysEx_lastChunkSize != 0) sysEx_chunks++;
    sysEx_chunkCount = 0;
    midiInfo(DONE_USBMIDI_CONFIG_ALLOC);
  }
  else {
    if (sysEx_chunks == 1) { // Only one chunk to load
      memcpy(sysEx_chunk_ptr, data_ptr + 2, sysEx_dataSize);
    }
    else if (sysEx_chunkCount == 0 && sysEx_chunks > 1) { // First chunk
      sysEx_chunkSize = USB_MIDI_SYSEX_MAX - 2; // Removing header size
      memcpy(sysEx_chunk_ptr, data_ptr + 2, sysEx_chunkSize);
      sysEx_chunk_ptr += sysEx_chunkSize;
      sysEx_chunkCount++;
    }
    else if (sysEx_chunkCount < sysEx_chunks - 1){ // Middle chunks
      sysEx_chunkSize = USB_MIDI_SYSEX_MAX;
      memcpy(sysEx_chunk_ptr, data_ptr, sysEx_chunkSize);
      sysEx_chunk_ptr += sysEx_chunkSize;
      sysEx_chunkCount++;
    }
    else { // Last chunk
      sysEx_chunkSize = sysEx_lastChunkSize - 1; // Removing footer size
      memcpy(sysEx_chunk_ptr, data_ptr, sysEx_chunkSize);

      if (sysEx_identifier == SYSEX_CONF) {
        #if defined(DEBUG_CONFIG)
          printBytes(sysEx_data_ptr, sysEx_dataSize);
        #endif
        load_config(sysEx_data_ptr, DONE_USBMIDI_CONFIG_LOAD);
        sysEx_alloc = true;
      }
      else if (sysEx_identifier == SYSEX_SOUND){
        // TODO
        //midiInfo(DONE_USBMIDI_SOUND_LOAD);
        //set_state(DONE_ACTION);
        sysEx_alloc = true;
      }
      else {
        midiInfo(ERROR_UNKNOWN_SYSEX);
        set_state(ERROR);
        sysEx_alloc = true;
      };
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

uint8_t blobValues[6] = {0}; 

void usb_midi_transmit(void) {
  static uint32_t usbTransmitTimeStamp = 0;

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
        //USB_MIDI_SYSEX_MAX = 290;
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
              /*
              // usbMIDI.sendControlChange(control, value, channel);
              usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0, WIDTH, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0, HEIGHT, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BZ, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
              usbMIDI.send_now();
              */
              blobValues[0] = blob_ptr->UID + 1;
              blobValues[1] = (uint8_t)round(map(blob_ptr->centroid.X, 0, WIDTH, 0, 127));
              blobValues[2] = (uint8_t)round(map(blob_ptr->centroid.Y, 0, HEIGHT, 0, 127));
              blobValues[3] = constrain(blob_ptr->centroid.Z, 0, 127);
              blobValues[4] = blob_ptr->box.W;
              blobValues[5] = blob_ptr->box.H;
              usbMIDI.sendSysEx(6, blobValues);
              //usbMIDI.send_now(); 
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