/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "usb_serial_transmit.h"

void USB_SERIAL_TRANSMIT_SETUP(void) {
  Serial.begin(BAUD_RATE);
  //while (!Serial);
  Serial.printf("\n%s_%s_%s", NAME, PROJECT, VERSION);
  Serial.printf("\nCURRENT_MODE:\t%d", currentMode);
};