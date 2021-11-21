/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_MIDI_TRANSMIT_H__
#define __USB_MIDI_TRANSMIT_H__

void USB_MIDI_TRANSMIT_SETUP(void);
void usb_midi_read_input(void);
void usb_midi_transmit(void);

#endif /*__USB_MIDI_TRANSMIT_H__*/