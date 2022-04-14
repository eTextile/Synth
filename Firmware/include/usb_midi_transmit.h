/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_MIDI_TRANSMIT_H__
#define __USB_MIDI_TRANSMIT_H__

#include <Arduino.h>

extern uint32_t bootTime;

extern uint16_t sysEx_dataSize;
extern uint8_t* sysEx_data_ptr;

void usb_midi_transmit_setup(void);

void midiInfo(uint8_t msg, uint8_t channel);
void usb_midi_pending_mode_timeout(void);

void usb_midi_read_input(void);
void usb_midi_transmit(void);

#endif /*__USB_MIDI_TRANSMIT_H__*/