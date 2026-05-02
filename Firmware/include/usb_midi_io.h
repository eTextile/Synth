/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_MIDI_IO_H__
#define __USB_MIDI_IO_H__

#include <Arduino.h>
#include "midi_bus.h"

extern uint32_t boot_time;
extern size_t sysEx_data_length;
extern uint8_t* sysEx_data_ptr;

void usb_midi_setup(void);
void usb_midi_receive(void);

void usb_midi_transmit_raw_matrix(void);
void usb_midi_transmit_interp_matrix(void);
void usb_midi_transmit_blobs(void);
void usb_midi_transmit_mappings_midi_msg(void);

void usb_midi_send_info(uint8_t msg, uint8_t channel);

void usb_read_note_on(uint8_t channel, uint8_t note, uint8_t velocity);
void usb_read_note_off(uint8_t channel, uint8_t note, uint8_t velocity);
void usb_read_control_change(uint8_t channel, uint8_t control, uint8_t value);
void usb_read_after_touch_poly(uint8_t channel, uint8_t note, uint8_t pressure);
void usb_read_pitch_bend(uint8_t channel, int pitch);
void usb_read_program_change(uint8_t channel, uint8_t program);
void usb_read_system_exclusive(const uint8_t* data_ptr, uint16_t length, bool complete);

#endif /*__USB_MIDI_IO_H__*/
