/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "midi_transmit.h"

#include <MIDI.h>                           // http://www.pjrc.com/teensy/td_midi.html

unsigned long int transmitTimer = 0;
#define TRANSMIT_INTERVAL 3

#if MIDI_HARDWARE
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);
#endif

#define MIDI_NODES 32

midiNode_t midiNodeArray[MIDI_NODES] = {0}; // Memory allocation for all MIDI I/O messages

llist_t midi_node_stack;                    // Main MIDI node stack
llist_t midiIn;                             // Main MIDI Input linked list
llist_t midiOut;                            // Main MIDI Output linked list

void llist_midi_init(llist_t* llist_ptr, midiNode_t* nodeArray_ptr, const int nodes) {
  llist_raz(llist_ptr);
  for (int i = 0; i < nodes; i++) {
    llist_push_front(llist_ptr, &nodeArray_ptr[i]);
  }
}

void MIDI_TRANSMIT_SETUP(void) {
#if MIDI_USB
  usbMIDI.begin();
  //usbMIDI.setHandleProgramChange(handle_usb_midi_input_pc);
  usbMIDI.setHandleControlChange(handle_usb_midi_input_cc);
  //usbMIDI.setHandleNoteOff(handle_usb_midi_input_noteOn);
  //usbMIDI.setHandleNoteOn(handle_usb_midi_input_noteOff);
#endif
#if MIDI_HARDWARE
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleControlChange(handle_hardware_midi_input_cc);
  MIDI.setHandleNoteOff(handle_hardware_midi_input_noteOn);
  MIDI.setHandleNoteOn(handle_hardware_midi_input_noteOff);
#endif
  llist_midi_init(&midi_node_stack, &midiNodeArray[0], MIDI_NODES); // Add X nodes to the midi_node_stack
  llist_raz(&midiIn);
  llist_raz(&midiOut);
};

#if MIDI_USB
void read_usb_midi_input(void) {
  usbMIDI.read(1);        // Is there a MIDI incoming messages on channel One
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
};
#endif

#if MIDI_HARDWARE
void read_hardware_midi_input(void) {
  MIDI.read(1);           // Is there a MIDI incoming messages on channel One
  while (MIDI.read());    // Read and discard any incoming MIDI messages
};
#endif

void handle_usb_midi_input_cc(byte channel, byte control, byte value) {
  switch (control) {
    case THRESHOLD:   // PROGRAM 3
      //lastMode = currentMode;
      currentMode = THRESHOLD;
      presets[THRESHOLD].val = map(value, 0, 127, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
      encoder.write(presets[THRESHOLD].val << 2);
      interpThreshold = constrain(presets[THRESHOLD].val - 5, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
      presets[THRESHOLD].ledVal = map(value, 0, 127, 0, 255);
      presets[THRESHOLD].setLed = true;
      presets[THRESHOLD].updateLed = true;
      presets[THRESHOLD].update = true;
      break;
    case CALIBRATE:   // PROGRAM 4
      lastMode = currentMode;
      currentMode = CALIBRATE;
      presets[CALIBRATE].setLed = true;
      break;
    case MIDI_RAW:    // PROGRAM 8
      if (value == 1) {
        currentMode = MIDI_RAW;
      } else {
        currentMode = MIDI_OFF;
      };
      break;
    case MIDI_INTERP: // PROGRAM 9
      currentMode = MIDI_INTERP;
      break;
    case MIDI_BLOBS_PLAY: // PROGRAM 6
      currentMode = MIDI_BLOBS_PLAY;
      break;
    case MIDI_BLOBS_LEARN: // PROGRAM 7
      currentMode = MIDI_BLOBS_LEARN;
      break;
    case MIDI_MAPPING: // PROGRAM 8
      currentMode = MIDI_MAPPING;
      break;
    default:
      break;
  };
};

void handle_hardware_midi_input_cc(byte channel, byte control, byte value) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.status = MIDI_CONTROL_CHANGE;                         // Set the MIDI status
  node_ptr->midiMsg.data1 = control;                                      // Set the MIDI control
  node_ptr->midiMsg.data2 = value;                                        // Set the MIDI value
  node_ptr->midiMsg.channel = channel;                                    // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);                                    // Add the node to the midiIn linked liste
}

void handle_hardware_midi_input_noteOn(byte channel, byte note, byte velocity) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.status = MIDI_NOTE_ON;                                // Set the MIDI status
  node_ptr->midiMsg.data1 = note;                                         // Set the MIDI note
  node_ptr->midiMsg.data2 = velocity;                                     // Set the MIDI velocity
  node_ptr->midiMsg.channel = channel;                                    // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);                                    // Add the node to the midiIn linked liste
};

void handle_hardware_midi_input_noteOff(byte channel, byte note, byte velocity) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.status = MIDI_NOTE_OFF;                               // Set the MIDI status
  node_ptr->midiMsg.data1 = note;                                         // Set the MIDI note
  node_ptr->midiMsg.data2 = velocity;                                     // Set the MIDI velocity
  node_ptr->midiMsg.channel = channel;                                    // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);                                    // Add the node to the midiIn linked liste
}

#if MIDI_USB || MIDI_HARDWARE
void midi_transmit(void) {
  switch (currentMode) {
    case MIDI_RAW:
      if (millis() - transmitTimer > TRANSMIT_INTERVAL) {
        transmitTimer = millis();
        usbMIDI.sendSysEx(RAW_FRAME, rawFrame.pData, false, 0);
        usbMIDI.send_now();
      }
      break;
    case MIDI_INTERP:
      // NOT_WORKING See https://forum.pjrc.com/threads/28282-How-big-is-the-MIDI-receive-buffer
      //usbMIDI.sendSysEx(NEW_FRAME, interpFrame.pData, false, 0);
      //usbMIDI.send_now();
      break;
    case MIDI_BLOBS_PLAY:
      // Send all blobs values using MIDI format
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (blob_ptr->state) {
          if (!blob_ptr->lastState) {
            usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, 1); // sendNoteOn(note, velocity, channel);
          }
          else {
            if (millis() - transmitTimer > TRANSMIT_INTERVAL) {
              transmitTimer = millis();
              // usbMIDI.sendControlChange(control, value, channel);
              usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BD, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
            };
          };
        }
        else {
          usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, 1); // sendNoteOff(note, velocity, channel);
        };
        usbMIDI.send_now();
      };
      break;
    case MIDI_BLOBS_LEARN:
      // Send separate blobs values using Control Change MIDI format
      // Send only the last blob that have been added to the sensor surface
      // Select blob's values according to the encoder position to allow the auto-mapping into Max4Live...
      blob_t* blob_ptr = (blob_t*)blobs.tail_ptr;
      if (blob_ptr != NULL) {
        switch (presets[MIDI_BLOBS_LEARN].val) {
          case BS:
            //usbMIDI.sendControlChange(BS, blob_ptr->state, blob_ptr->UID + 1);
            if (!blob_ptr->lastState) usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, 0);
            if (!blob_ptr->state) usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, 0);
            break;
          case BX:
            usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, X_MAX - X_MIN , 0, 127)), blob_ptr->UID + 1);
            break;
          case BY:
            usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
            break;
          case BW:
            usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
            break;
          case BH:
            usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
            break;
          case BD:
            usbMIDI.sendControlChange(BD, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
            break;
          default:
            break;
        };
      };
      break;
    case MIDI_MAPPING:
      for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
        switch (node_ptr->midiMsg.status) {
          case MIDI_NOTE_ON:
#if MIDI_USB
            usbMIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);        // Send MIDI noteOn
#endif
#if MIDI_HARDWARE
            MIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);           // Send MIDI noteOn
#endif
            break;
          case MIDI_NOTE_OFF:
#if MIDI_USB
            usbMIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);       // Send MIDI noteOff
#endif
#if MIDI_HARDWARE
            MIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);          // Send MIDI noteOff
#endif
            break;
          case MIDI_CONTROL_CHANGE:
#if MIDI_USB
            usbMIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel); // Send MIDI noteOff
#endif
#if MIDI_HARDWARE
            MIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);    // Send MIDI noteOff
#endif
            break;
          default:
            break;
        };
      };
      usbMIDI.send_now();
      llist_save_nodes(&midi_node_stack, &midiOut); // Save all midiOut nodes to the midi_node_stack linked list
      break;
    default:
      break;
  };
};
#endif
