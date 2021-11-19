/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
  See: https://github.com/makeabilitylab/p5js/tree/master/WebSerial/Basic
  Demo: https://makeabilitylab.github.io/p5js/WebSerial/Basic/SliderOut/
*/

#include "usb_serial_transmit.h"

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"

#include <SerialFlash.h> // https://github.com/PaulStoffregen/SerialFlash
#include <SPI.h>

#define SERIAL_UPDATE_CONFIG_TIMEOUT 4000
uint32_t serialUpdateConfigTimeStamp = 0;

#define SERIAL_TRANSMIT_INTERVAL 10
uint32_t serialTransmitTimeStamp = 0;

// Buffer sizes
#define USB_BUFFER_SIZE       128
#define FLASH_BUFFER_SIZE     4096

// Max filename length (8.3 plus a null char terminator)
#define FILENAME_STRING_SIZE  8

// State machine
#define STATE_START           0
#define STATE_SIZE            1
#define STATE_CONTENT         2

// Special bytes in the communication protocol
#define BYTE_START            0x7e
#define BYTE_ESCAPE           0x7d
#define BYTE_SEPARATOR        0x7c

static void error(void) {
  while (1) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(5);
    digitalWrite(LED_BUILTIN, LOW);
    delay(5);
  };
};

void flushError() {
  uint32_t lastReceiveTime = millis();
  char usbBuffer[USB_BUFFER_SIZE];
  // We assume the serial receive part is finished when we have not received something for 3 seconds
  while (Serial.available() || lastReceiveTime + 3000 > millis()) {
    if (Serial.readBytes(usbBuffer, USB_BUFFER_SIZE)) {
      lastReceiveTime = millis();
      serialUpdateConfigTimeStamp = millis();
    };
  };
};

void USB_SERIAL_TRANSMIT_SETUP(void) {
  Serial.begin(BAUD_RATE);
};

void USB_SERIAL_UPDATE_CONFIG(void) {

  pinMode(LED_BUILTIN, OUTPUT);

  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    error();
  };

  // We start by formatting the flash...
  uint8_t id[5];
  SerialFlash.readID(id);
  SerialFlash.eraseAll();

  // Flash LED at 1Hz while formatting
  while (!SerialFlash.ready()) {
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
  };

  // Quickly flash LED a few times when completed, then leave the light on solid
  for (uint8_t i = 0; i < 10; i++) {
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }
  digitalWrite(LED_BUILTIN, HIGH);

  // We are now going to wait for the upload program
  if (millis() - serialUpdateConfigTimeStamp < SERIAL_UPDATE_CONFIG_TIMEOUT) {
    if (!Serial.available()) {
      // Waiting for config file!
    } else {

      SerialFlashFile flashFile;

      uint8_t state = STATE_START;
      uint8_t escape = 0;
      uint8_t fileSizeIndex = 0;
      uint32_t fileSize = 0;
      char filename[FILENAME_STRING_SIZE];

      char usbBuffer[USB_BUFFER_SIZE];
      uint8_t flashBuffer[FLASH_BUFFER_SIZE];

      uint16_t flashBufferIndex = 0;
      uint8_t filenameIndex = 0;

      uint32_t lastReceiveTime = millis();

      // We assume the serial receive part is finished when we have not received something for 3 seconds
      while (Serial.available() || lastReceiveTime + 3000 > millis()) {
        uint16_t available = Serial.readBytes(usbBuffer, USB_BUFFER_SIZE);
        if (available) {
          lastReceiveTime = millis();
          serialUpdateConfigTimeStamp = millis();
        };

        for (uint16_t usbBufferIndex = 0; usbBufferIndex < available; usbBufferIndex++) {
          uint8_t b = usbBuffer[usbBufferIndex];

          if (state == STATE_START) {
            // Start byte - Reepat start is fine
            if (b == BYTE_START) {
              for (uint8_t i = 0; i < FILENAME_STRING_SIZE; i++) {
                filename[i] = 0x00;
              }
              filenameIndex = 0;
            }
            // Valid characters are A-Z, 0-9, comma, period, colon, dash, underscore
            else if ((b >= 'A' && b <= 'Z') || (b >= '0' && b <= '9') || b == '.' || b == ',' || b == ':' || b == '-' || b == '_') {
              filename[filenameIndex++] = b;
              if (filenameIndex >= FILENAME_STRING_SIZE) {
                //Error name too long
                flushError();
                return;
              }
            }
            // Filename end character
            else if (b == BYTE_SEPARATOR) {
              if (filenameIndex == 0) {
                // Error empty filename
                flushError();
                return;
              }

              // Change state
              state = STATE_SIZE;
              fileSizeIndex = 0;
              fileSize = 0;
            }
            // Invalid character
            else {
              // Error bad filename
              flushError();
              return;
            };
          }
          // We read 4 bytes as a uint32_t for file size
          else if (state == STATE_SIZE) {
            if (fileSizeIndex < 4) {
              fileSize = (fileSize << 8) + b;
              fileSizeIndex++;
            }
            else if (b == BYTE_SEPARATOR) {
              state = STATE_CONTENT;
              flashBufferIndex = 0;
              escape = 0;

              if (SerialFlash.exists(filename)) {
                SerialFlash.remove(filename);  // It doesn't reclaim the space, but it does let you create a new file with the same name
              };

              // Create a new file and open it for writing
              if (SerialFlash.create(filename, fileSize)) {
                flashFile = SerialFlash.open(filename);
                if (!flashFile) {
                  // Error flash file open
                  flushError();
                  return;
                }
              }
              else {
                // Error flash create (no room left?)
                flushError();
                return;
              };
            }
            else {
              // Error invalid length requested
              flushError();
              return;
            };
          }
          else if (state == STATE_CONTENT) {
            // Previous byte was escaped; unescape and add to buffer
            if (escape) {
              escape = 0;
              flashBuffer[flashBufferIndex++] = b ^ 0x20;
            }
            // Escape the next byte
            else if (b == BYTE_ESCAPE) {
              //Serial.println("esc");
              escape = 1;
            }
            // End of file
            else if (b == BYTE_START) {
              //Serial.println("End of file");
              state = STATE_START;
              flashFile.write(flashBuffer, flashBufferIndex);
              flashFile.close();
              flashBufferIndex = 0;
            }
            // Normal byte; add to buffer
            else {
              flashBuffer[flashBufferIndex++] = b;
            }

            // The buffer is filled; write to SD card
            if (flashBufferIndex >= FLASH_BUFFER_SIZE) {
              flashFile.write(flashBuffer, FLASH_BUFFER_SIZE);
              flashBufferIndex = 0;
            };
          };
        };
      };
      // Success!  Turn the light off.
      digitalWrite(LED_BUILTIN, LOW);
    };
  };
};

void usb_serial_handle_input(void) {
  if (Serial.available() > 0) {
    // If we're here, then serial data has been received
    // Read data off the serial port until we get to the endline delimiter ('\n')
    // Store all of this data into a string
    String rcvdSerialData = Serial.readStringUntil('\n');
    int ledValue = rcvdSerialData.toInt();
    analogWrite(LED_BUILTIN, ledValue);
  };
};

void usb_serial_transmit(void) {
  switch (currentMode) {
    case RAW_MATRIX:
      /*
        if (millis() - oscTransmitTimeStamp > OSC_TRANSMIT_INTERVAL) {
        oscTransmitTimeStamp = millis();
        OSCMessage msg("/RAW");
        msg.add(rawFrame.pData, RAW_FRAME);
        SLIPSerial.beginPacket();
        msg.send(SLIPSerial);
        SLIPSerial.endPacket();
        };
      */
      break;
    case INTERP_MATRIX:
      /*
        if (millis() - oscTransmitTimeStamp > OSC_TRANSMIT_INTERVAL) {
        oscTransmitTimeStamp = millis();
        OSCMessage msg("/INTERP");
        msg.add(interpFrame.pData, NEW_FRAME);
        SLIPSerial.beginPacket();
        msg.send(SLIPSerial);
        SLIPSerial.endPacket();
        };
      */
      break;
    case BLOBS_PLAY:
      for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
        if (blob_ptr->state) {
          if (!blob_ptr->lastState && blob_ptr->status == FREE) {
#if defined(DEBUG_SERIAL_TRANSMIT)
            Serial.printf("\nDEBUG_SERIAL_TRANSMIT: ON_%d", blob_ptr->UID);
#else
            OSCMessage msg("/ON");
            msg.add(blob_ptr->UID);
            SLIPSerial.beginPacket(); // Send SLIP header
            msg.send(SLIPSerial);     // Send the OSC bundle
            SLIPSerial.endPacket();   // Send the SLIP end of packet
#endif
          } else {
            if (millis() - blob_ptr->transmitTimeStamp > SERIAL_TRANSMIT_INTERVAL) {
              blob_ptr->transmitTimeStamp = millis();
#if defined(DEBUG_SERIAL_TRANSMIT)
              Serial.printf("\nDEBUG_SERIAL_TRANSMIT: UPDATE_%d", blob_ptr->UID);
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
#if defined(DEBUG_SERIAL_TRANSMIT)
            Serial.printf("\nDEBUG_SERIAL_TRANSMIT: OFF_%d", blob_ptr->UID);
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