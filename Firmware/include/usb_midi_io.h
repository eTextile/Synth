/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_MIDI_IO_H__
#define __USB_MIDI_IO_H__

#include <Arduino.h>

extern uint32_t boot_time;
extern size_t sysEx_data_length;
extern uint8_t* sysEx_data_ptr;

void usb_midi_setup(void);
void usb_midi_receive(void);

void usb_midi_transmit_raw_matrix(void);
void usb_midi_transmit_interp_matrix(void);
void usb_midi_transmit_blobs(void);
void mapping_usb_midi_transmit(void);

void usb_midi_send_clock(void);
void usb_midi_send_info(uint8_t msg, uint8_t channel);


#endif /*__USB_MIDI_IO_H__*/
