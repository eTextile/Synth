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

#define MIDI_NODES 64

midiNode_t midiNodeArray[MIDI_NODES] = {0}; // Memory allocation for all MIDI I/O messages

llist_t midi_node_stack;                    // Main MIDI node stack
llist_t midiIn;                             // Main MIDI Input linked list
llist_t midiOut;                            // Main MIDI Output linked list
llist_t midiChord;                          // Main MIDI chord linked list

void llist_midi_init(llist_t* llist_ptr, midiNode_t* nodeArray_ptr, const int nodes) {
  llist_raz(llist_ptr);
  for (int i = 0; i < nodes; i++) {
    llist_push_front(llist_ptr, &nodeArray_ptr[i]);
  };
};

void MIDI_TRANSMIT_SETUP(void) {
#if MIDI_USB
  usbMIDI.begin();
  usbMIDI.setHandleNoteOff(handle_midi_input_noteOn);
  usbMIDI.setHandleNoteOn(handle_midi_input_noteOff);
  usbMIDI.setHandleControlChange(handle_midi_input_cc);
#endif
#if MIDI_HARDWARE
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOff(handle_midi_input_noteOn);
  MIDI.setHandleNoteOn(handle_midi_input_noteOff);
  MIDI.setHandleControlChange(handle_midi_input_cc);
#endif
  llist_midi_init(&midi_node_stack, &midiNodeArray[0], MIDI_NODES); // Add X nodes to the midi_node_stack
  llist_raz(&midiIn);
  llist_raz(&midiOut);
  llist_raz(&midiChord);
};

void read_midi_input(void) {
#if MIDI_USB
  usbMIDI.read(1);        // Is there a MIDI incoming messages on channel One
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
#endif
#if MIDI_HARDWARE
  MIDI.read(1);           // Is there a MIDI incoming messages on channel One
  while (MIDI.read());    // Read and discard any incoming MIDI messages
#endif
};

void handle_usb_midi_input_cc(byte channel, byte control, byte value) {
  switch (control) {
    case THRESHOLD: // PROGRAM 3
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
    case CALIBRATE: // PROGRAM 4
      lastMode = currentMode;
      currentMode = CALIBRATE;
      presets[CALIBRATE].setLed = true;
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
    case MIDI_RAW: // PROGRAM 9
      if (value == 1) {
        currentMode = MIDI_RAW;
      } else {
        currentMode = MIDI_OFF;
      };
      break;
    case MIDI_INTERP: // PROGRAM 10
      currentMode = MIDI_INTERP;
      break;
    default:
      break;
  };
};

void handle_midi_input_cc(byte channel, byte control, byte value) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.status = MIDI_CONTROL_CHANGE;                         // Set the MIDI status
  node_ptr->midiMsg.data1 = control;                                      // Set the MIDI control
  node_ptr->midiMsg.data2 = value;                                        // Set the MIDI value
  node_ptr->midiMsg.channel = channel;                                    // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);                                    // Add the node to the midiIn linked liste
};

void handle_midi_input_noteOn(byte channel, byte note, byte velocity) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.status = MIDI_NOTE_ON;                                // Set the MIDI status
  node_ptr->midiMsg.data1 = note;                                         // Set the MIDI note
  node_ptr->midiMsg.data2 = velocity;                                     // Set the MIDI velocity
  node_ptr->midiMsg.channel = channel;                                    // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);                                    // Add the node to the midiIn linked liste
};

void handle_midi_input_noteOff(byte channel, byte note, byte velocity) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.status = MIDI_NOTE_OFF;                               // Set the MIDI status
  node_ptr->midiMsg.data1 = note;                                         // Set the MIDI note
  node_ptr->midiMsg.data2 = velocity;                                     // Set the MIDI velocity
  node_ptr->midiMsg.channel = channel;                                    // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);                                    // Add the node to the midiIn linked liste
};

void midi_transmit(void) {

  switch (currentMode) {

    case MIDI_RAW:
      if (millis() - transmitTimer > TRANSMIT_INTERVAL) {
        transmitTimer = millis();
#if MIDI_USB
        usbMIDI.sendSysEx(RAW_FRAME, rawFrame.pData, false, 0);
        usbMIDI.send_now();
#endif
      };
      break;

    case MIDI_INTERP:
      if (millis() - transmitTimer > TRANSMIT_INTERVAL) {
        transmitTimer = millis();
#if MIDI_USB
        // NOT_WORKING! See https://forum.pjrc.com/threads/28282-How-big-is-the-MIDI-receive-buffer
        //usbMIDI.sendSysEx(NEW_FRAME, interpFrame.pData, false, 0);
        //usbMIDI.send_now();
#endif
      };
      break;

    case MIDI_BLOBS_PLAY:
      // Send all blobs values using MIDI format
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (blob_ptr->state) {
          if (!blob_ptr->lastState) {
#if MIDI_USB
            usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, 1); // sendNoteOn(note, velocity, channel);
#endif
          }
          else {
            if (millis() - transmitTimer > TRANSMIT_INTERVAL) {
              transmitTimer = millis();
              // usbMIDI.sendControlChange(control, value, channel);
#if MIDI_USB
              usbMIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BW, blob_ptr->box.W, blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BH, blob_ptr->box.H, blob_ptr->UID + 1);
              usbMIDI.sendControlChange(BD, constrain(blob_ptr->centroid.Z, 0, 127), blob_ptr->UID + 1);
#endif
            };
          };
        }
        else {
#if MIDI_USB
          usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, 1); // sendNoteOff(note, velocity, channel);
#endif
        };
        //usbMIDI.send_now();
      };
      break;

    case MIDI_BLOBS_LEARN:
      // Send separate blobs values using Control Change MIDI format
      // Send only the last blob that have been added to the sensor surface
      // Select blob's values according to the encoder position to allow the auto-mapping into Max4Live...
      if ((blob_t*)blobs.tail_ptr != NULL) {

        blob_t* blob_ptr = (blob_t*)blobs.tail_ptr; // FIXME!

        switch (presets[MIDI_BLOBS_LEARN].val) {
#if MIDI_USB
          case BS:
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
#endif
          default:
            break;
        };
      };
      break;

    case MIDI_MAPPING:
      for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
        switch (node_ptr->midiMsg.status) {
          case MIDI_NOTE_ON:
#if DEBUG_MIDI_TRANSMIT
            Serial.printf("\nTRANSMIT\tNOTE_ON:%d\tVALUE:%d\tCHANNEL:%d", node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_CHANNEL);
#endif
#if MIDI_USB
            usbMIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_CHANNEL);        // USB send MIDI noteOn
#endif
#if MIDI_HARDWARE
            MIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_CHANNEL);           // Hardware send MIDI noteOn
#endif
            break;
          case MIDI_NOTE_OFF:
#if DEBUG_MIDI_TRANSMIT
            Serial.printf("\nTRANSMIT\tNOTE_OFF:%d\tVALUE:%d\tCHANNEL:%d", node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_CHANNEL);
#endif
#if MIDI_USB
            usbMIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_CHANNEL);       // USB send MIDI noteOff
#endif
#if MIDI_HARDWARE
            MIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_CHANNEL);          // Hardware send MIDI noteOff
#endif
            break;
          case MIDI_CONTROL_CHANGE:
#if DEBUG_MIDI_TRANSMIT
            Serial.printf("\nTRANSMIT\tCC:%d\tVALUE:%d\tCHANNEL:%d", node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_CHANNEL);
#endif
#if MIDI_USB
            usbMIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_CHANNEL); // USB send MIDI control_change
#endif
#if MIDI_HARDWARE
            MIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, MIDI_CHANNEL);    // Hardware send MIDI control_change
#endif
            break;
          default:
            break;
        };
      };
#if MIDI_USB
      usbMIDI.send_now();
#endif
#if MIDI_HARDWARE
      //MIDI.send();
#endif
      llist_save_nodes(&midi_node_stack, &midiOut); // Save/rescure all midiOut nodes
      break;

    default:
      break;
  };
};
