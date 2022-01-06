/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __HARDWARE_MIDI_TRANSMIT_H__
#define __HARDWARE_MIDI_TRANSMIT_H__

#include "midi_bus.h"

void HARDWARE_MIDI_TRANSMIT_SETUP(void);

void hardware_midi_read_input(void);
void hardware_midi_handle_input(const midi::Message<128u> &midiMsg);
void hardware_midi_transmit(void);

#endif /*__HARDWARE_MIDI_TRANSMIT_H__*/