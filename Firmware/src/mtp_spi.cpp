/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mtp_spi.h"

#if defined(USB_MTPDISK) || defined(USB_MTPDISK_MIDI) || defined(USB_MTPDISK_SERIAL)

#include "config.h"
#include "mapping_lib.h"

#include <FS.h>
#include <LittleFS.h>
#include <MTP_Teensy.h>

char jsonFile[1024] = {0};

LittleFS_SPIFlash myfs;
File dataFile;

bool write_data = false;
uint32_t diskSize;

MTPStorage storage;
MTPD mtpd(&storage);

void MTP_SPI_SETUP(void) {
  //Serial.begin(115200);
  if (!myfs.begin(FLASH_CHIP_SELECT, SPI)) {
    while (1){
        digitalWrite(LED_PIN_D1, HIGH);
        digitalWrite(LED_PIN_D2, HIGH);
        delay(5);
        digitalWrite(LED_PIN_D1, LOW);
        digitalWrite(LED_PIN_D2, LOW);
        delay(5);
    };
  };
  mtpd.begin();
  storage.addFilesystem(myfs, "E256");
  dump_json();
};

void handle_mtp_spi(void) {
    mtpd.loop();
    //if (write_data) write_start();
};

void write_start(void) {
  String dataString = "";

  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    };
  };
  //Serial.begin(115200);
  //while (!Serial);
  if (dataFile) {
    dataFile.println(dataString);
    //Serial.println(dataString);
  } else {
      digitalWrite(LED_PIN_D1, HIGH);
      digitalWrite(LED_PIN_D2, HIGH);
      delay(5);
      digitalWrite(LED_PIN_D1, LOW);
      digitalWrite(LED_PIN_D2, LOW);
      delay(5);  
    };
  delay(100);
};

void write_stop(void) {
  write_data = false;
  dataFile.close();
  //mtpd.send_DeviceResetEvent();
};

void dump_json(void) {
  dataFile = myfs.open("config.json");
  if (dataFile) {
    while (dataFile.available()) {
      dataFile.read(jsonFile, 1024);
    };
    dataFile.close();
    return;
  }
  else {
    digitalWrite(LED_PIN_D1, HIGH);
    digitalWrite(LED_PIN_D2, HIGH);
    delay(5);
    digitalWrite(LED_PIN_D1, LOW);
    digitalWrite(LED_PIN_D2, LOW);
    delay(5);  
  };
};

#endif