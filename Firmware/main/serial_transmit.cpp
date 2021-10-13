/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "serial_transmit.h"

unsigned long int transmitSerialTimer = 0;
#define TRANSMIT_INTERVAL 5

void SERIAL_TRANSMIT_SETUP(void) {
  Serial.begin(BAUD_RATE);
  while (!Serial);
  Serial.printf("\n%s_%s_%s", NAME, PROJECT, VERSION);
  Serial.printf("\nCURRENT_MODE_:_%d", currentMode);
};

void read_serial_input(void) {
  if (Serial.available()) {
    midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);  // Get a node from the MIDI nodes stack
    node_ptr->midiMsg.status = MIDI_CONTROL_CHANGE;                         // Set the MIDI status
    node_ptr->midiMsg.data1 = Serial.read();                                // Set the MIDI control
    node_ptr->midiMsg.data2 = 0;                                            // Set the MIDI value
    node_ptr->midiMsg.channel = MIDI_INPUT_CHANNEL;                         // Set the MIDI channel
    llist_push_front(&midiIn, node_ptr);                                    // Add the node to the midiIn linked liste
  };
};

void serial_transmit(void) {
  switch (currentMode) {
    case SERIAL_RAW:
      // Send all matrix raw values using SERIAL line
      if (millis() - transmitSerialTimer > TRANSMIT_INTERVAL) {
        transmitSerialTimer = millis();
        Serial.write(rawFrame.pData, RAW_FRAME);
        Serial.write(255);
      };
      break;
    case SERIAL_INTERP:
      // Send all matrix interpolated values using SERIAL line
      if (millis() - transmitSerialTimer > TRANSMIT_INTERVAL) {
        transmitSerialTimer = millis();
        Serial.write(interpFrame.pData, NEW_FRAME);
        Serial.write(255);
      };
      break;
    case SERIAL_BLOBS:
      // Send all blobs values using SERIAL line
      uint8_t serialPaket[8] = {0};
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (millis() - blob_ptr->timeTag_transmit > TRANSMIT_INTERVAL) {
          blob_ptr->timeTag_transmit = millis();
          serialPaket[0] = blob_ptr->UID;
          serialPaket[1] = blob_ptr->state;
          serialPaket[2] = blob_ptr->lastState;
          serialPaket[3] = (uint8_t)round(map(blob_ptr->centroid.X, 0, X_MAX - X_MIN, 0, 127));
          serialPaket[4] = (uint8_t)round(map(blob_ptr->centroid.Y, 0, X_MAX - X_MIN, 0, 127));
          serialPaket[5] = constrain(blob_ptr->centroid.Z, 0, 127);
          serialPaket[6] = blob_ptr->box.W;
          serialPaket[7] = blob_ptr->box.H;
          Serial.write(serialPaket, 8);
          Serial.write(255);
        };
      };
      break;
    default:
      break;
  };
};
