/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "usb_midi_transmit.h"
#include "config.h"
#include "scan.h"
#include "llist.h"
#include "blob.h"
#include "midi_bus.h"
#include "allocate.h"
#include <ArduinoJson.h>

#define MIDI_TRANSMIT_INTERVAL 500
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

inline void usb_set_level(e256_control_t *ctr_ptr, uint8_t levelMode, uint8_t value) {
  setup_leds(ctr_ptr->levels);
  ctr_ptr->levels[playMode].leds.update = false;
  ctr_ptr->levels[levelMode].val = value;
  ctr_ptr->encoder->write(ctr_ptr->levels[levelMode].val << 2);
  ctr_ptr->levels[levelMode].leds.setup = true;
  ctr_ptr->levels[levelMode].leds.update = true;
  ctr_ptr->levels[levelMode].run = true;
  #if defined(DEBUG_LEVELS)
    Serial.printf("\nDEBUG_USB_LEVEL_MODE:%d", (ctr_ptr->levels[levelMode].val << 2));
  #endif
};

void e256_controlChange(byte channel, byte control, byte value){
  usb_set_level(&e256_ctr, control, value);
};

void e256_programChange(byte channel, byte program){
  //set_mode(&e256_ctr, program);
};

// 
boolean load_config(char* data_ptr){

  StaticJsonDocument<2048> config;
  DeserializationError err = deserializeJson(config, data_ptr);

  if (err) {
    playMode = ERROR;
    usbMIDI.sendProgramChange(ERROR_WAITING_FOR_GONFIG, MIDI_OUTPUT_CHANNEL); // ProgramChange(program, channel);
    usbMIDI.send_now();
    #if defined(DEBUG_MIDI_CONFIG)
      Serial.printf("\nDEBUG_MIDI_CONFIG\tERROR_WAITING_FOR_GONFIG!\t%s", err.f_str());
    #endif
    return false;
  };
  if (!config_load_mapping(config["mapping"])) {
    playMode = ERROR;
    usbMIDI.sendProgramChange(ERROR_LOADING_GONFIG_FAILED, MIDI_OUTPUT_CHANNEL); // ProgramChange(program, channel);
    usbMIDI.send_now();
    #if defined(DEBUG_MIDI_CONFIG)
      Serial.printf("\nDEBUG_MIDI_CONFIG\tERROR_LOADING_GONFIG_FAILED!");
    #endif
    return true;
  };
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
void e256_systemExclusive(byte* data_ptr, unsigned int length){
  #if defined(DEBUG_MIDI_CONFIG)
    Serial.print("SysEx Message: ");
    printBytes(data_ptr, length);
    Serial.println();
  #endif
  char identifier = *data_ptr + (2*sizeof(byte));
  if (identifier == MAPPING_CONFIG){
    configLength = length - 2;
    config_ptr = allocate((char*)data_ptr, configLength);
    strcpy(config_ptr, (char*)data_ptr);
    load_config(config_ptr);
  };
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
              usbMIDI.send_now();
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