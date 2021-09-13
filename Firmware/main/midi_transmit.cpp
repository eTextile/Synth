/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "midi_transmit.h"

#include <MIDI.h>                           // http://www.pjrc.com/teensy/td_midi.html

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
  llist_midi_init(&midi_node_stack, &midiNodeArray[0], MAX_SYNTH); // Add X nodes to the midi_node_stack
  llist_raz(&midiIn);
  llist_raz(&midiOut);
};

void read_usb_midi_input(void) {
  usbMIDI.read(1);        // Is there a MIDI incoming messages on channel One
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
};

#if MIDI_HARDWARE
void read_hardware_midi_input(void) {
  MIDI.read(1);           // Is there a MIDI incoming messages on channel One
  while (MIDI.read());    // Read and discard any incoming MIDI messages
};
#endif

void handle_usb_midi_input_cc(byte channel, byte control, byte value) {
  switch (control) {
    case THRESHOLD:   // CONTROL 3
      lastMode = currentMode;
      currentMode = THRESHOLD;
      presets[THRESHOLD].val = map(value, 0, 127, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
      encoder.write(presets[THRESHOLD].val << 2);
      interpThreshold = constrain(presets[THRESHOLD].val - 5, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
      presets[THRESHOLD].setLed = true;
      presets[THRESHOLD].ledVal = map(value, 0, 127, 0, 255);
      presets[THRESHOLD].updateLed = true;
      presets[THRESHOLD].update = true;
      break;
    case CALIBRATE:   // CONTROL 4
      lastMode = currentMode;
      currentMode = CALIBRATE;
      presets[CALIBRATE].setLed = true;
      presets[CALIBRATE].updateLed = true;
      break;
    case MIDI_RAW:    // CONTROL 6
      if (value == 1) {
        lastMode = currentMode;
        currentMode = MIDI_RAW;
      } else {
        currentMode = MIDI_OFF;
      };
      break;
    case MIDI_INTERP: // CONTROL 7
      if (value == 1) {
        lastMode = currentMode;
        currentMode = MIDI_INTERP;
      } else {
        currentMode = MIDI_OFF;
      };
      break;
    case MIDI_BLOBS:  // CONTROL 8
      if (value == 1) {
        lastMode = currentMode;
        currentMode = MIDI_BLOBS;
      } else {
        currentMode = MIDI_OFF;
      };
      break;
    case MIDI_BLOBS_LEARN:  // CONTROL 9
      if (value == 1) {
        currentModeState = MIDI_BLOBS_LEARN;
      } else {
        currentModeState = MIDI_BLOBS_PLAY;
      };
      break;

    default:
      break;
  };
};

void handle_hardware_midi_input_cc(byte channel, byte control, byte value) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.status = midi::ControlChange;                         //
  node_ptr->midiMsg.data1 = control;                                      // Set the MIDI control
  node_ptr->midiMsg.data2 = value;                                        // Set the MIDI value
  node_ptr->midiMsg.channel = channel;                                    // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);                                    // Add the node to the midiIn linked liste
}

void handle_hardware_midi_input_noteOn(byte channel, byte note, byte velocity) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.status = midi::NoteOn;                                //
  node_ptr->midiMsg.data1 = note;                                         // Set the MIDI note
  node_ptr->midiMsg.data2 = velocity;                                     // Set the MIDI velocity
  node_ptr->midiMsg.channel = channel;                                    // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);                                    // Add the node to the midiIn linked liste
};

void handle_hardware_midi_input_noteOff(byte channel, byte note, byte velocity) {
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
  node_ptr->midiMsg.status = midi::NoteOff;                               //
  node_ptr->midiMsg.data1 = note;                                         // Set the MIDI note
  node_ptr->midiMsg.data2 = velocity;                                     // Set the MIDI velocity
  node_ptr->midiMsg.channel = channel;                                    // Set the MIDI channel
  llist_push_front(&midiIn, node_ptr);                                    // Add the node to the midiIn linked liste
}

void midi_transmit(void) {
  switch (currentMode) {
    case MIDI_RAW:
      usbMIDI.sendSysEx(RAW_FRAME, rawFrame.pData, false);
      usbMIDI.send_now();
      break;
    case MIDI_INTERP:
      usbMIDI.sendSysEx(NEW_FRAME, interpFrame.pData, false);
      usbMIDI.send_now();
      break;
    case MIDI_BLOBS:
      switch (currentMode) {
        case MIDI_BLOBS_LEARN:
          // Send blobs values using ControlChange MIDI format
          // Send only the last blob that have been added to the sensor surface
          // Separate blob's values according to the encoder position to allow the mapping into Max4Live
          blob_t* blob_ptr = (blob_t*)blobs.tail_ptr;
          if (blob_ptr != NULL) {
            switch (presets[MIDI_BLOBS].val) {
              case BS:
                //usbMIDI.sendControlChange(BS, blob_ptr->state, blob_ptr->UID + 1);
                if (!blob_ptr->lastState) usbMIDI.sendNoteOn(blob_ptr->UID + 1, 1, 0);
                if (!blob_ptr->state) usbMIDI.sendNoteOff(blob_ptr->UID + 1, 0, 0);
                break;
              case BL:
                usbMIDI.sendControlChange(BS, blob_ptr->lastState, blob_ptr->UID + 1);
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
            };
          };
          break;
        case MIDI_BLOBS_PLAY:
          // Send all blobs values using MIDI format
          for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
            if (blob_ptr->state) {
              if (!blob_ptr->lastState) {
                usbMIDI.sendNoteOn((int8_t)blob_ptr->UID + 1, 1, 0);
              }
              else {
                usbMIDI.sendControlChange(BX, (int8_t)round(map(blob_ptr->centroid.X, 0, X_MAX - X_MIN, 0, 127)), (int8_t)blob_ptr->UID + 1);
                usbMIDI.sendControlChange(BY, (int8_t)round(map(blob_ptr->centroid.Y, 0, X_MAX - X_MIN, 0, 127)), (int8_t)blob_ptr->UID + 1);
                usbMIDI.sendControlChange(BW, (int8_t)blob_ptr->box.W, (int8_t)(blob_ptr->UID + 1));
                usbMIDI.sendControlChange(BH, (int8_t)blob_ptr->box.H, (int8_t)(blob_ptr->UID + 1));
                usbMIDI.sendControlChange(BD, (int8_t)constrain(blob_ptr->centroid.Z, 0, 127), (int8_t)blob_ptr->UID + 1);

                //MIDI.sendControlChange(BX, (uint8_t)round(map(blob_ptr->centroid.X, 0.0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1); // Make it Q2
                //MIDI.sendControlChange(BY, (uint8_t)round(map(blob_ptr->centroid.Y, 0.0, X_MAX - X_MIN, 0, 127)), blob_ptr->UID + 1);
                //MIDI.sendControlChange(BW, (int8_t)blob_ptr->box.W, (int8_t)(blob_ptr->UID + 1));
                //MIDI.sendControlChange(BH, (int8_t)blob_ptr->box.H, (int8_t)(blob_ptr->UID + 1));
                //MIDI.sendControlChange(BD, (int8_t)constrain(blob_ptr->centroid.Z, 0, 127), (int8_t)blob_ptr->UID + 1);
              };
            }
            else {
              usbMIDI.sendNoteOff((int8_t)blob_ptr->UID + 1, 0, 0);
            };
          };
          break;
        case MIDI_MAPPING:
          for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiOut); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
            switch (node_ptr->midiMsg.status) {
              case midi::NoteOn:
#if MIDI_USB
                usbMIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);  // Send MIDI noteOn
#endif
#if MIDI_HARDWARE
                MIDI.sendNoteOn(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);     // Send MIDI noteOn
#endif
                break;
              case midi::NoteOff:
#if MIDI_USB
                usbMIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel); // Send MIDI noteOff
#endif
#if MIDI_HARDWARE
                MIDI.sendNoteOff(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);    // Send MIDI noteOff
#endif
                break;
              case midi::ControlChange:
#if MIDI_USB
                usbMIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel); // Send MIDI noteOff
#endif
#if MIDI_HARDWARE
                MIDI.sendControlChange(node_ptr->midiMsg.data1, node_ptr->midiMsg.data2, node_ptr->midiMsg.channel);    // Send MIDI noteOff
#endif
                break;
            };
          };
          llist_save_nodes(&midi_node_stack, &midiOut); // Save all midiOut nodes to the midi_node_stack linked list
      };
  };
};
