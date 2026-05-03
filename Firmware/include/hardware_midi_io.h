/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __HARDWARE_MIDI_IO_H__
#define __HARDWARE_MIDI_IO_H__

#include "midi_bus.h"

extern uint8_t hardware_midi_input_channel; // runtime-configurable, set from JSON config

void hardware_midi_setup(void);
void hardware_midi_receive(void);
void hardware_midi_set_input_channel(uint8_t channel);
void hardware_midi_handle_input(const Message<128u> &midi_msg);
void hardware_midi_transmit_mappings_midi_msg(void);
void hardware_midi_send_clock(void);

#endif /*__HARDWARE_MIDI_IO_H__*/
