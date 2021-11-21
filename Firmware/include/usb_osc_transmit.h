/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_OSC_TRANSMIT_H__
#define __USB_OSC_TRANSMIT_H__

#if defined(USB_OSC) || defined(USB_MTPDISK)

#include <OSCMessage.h>             // https://github.com/CNMAT/OSC

void USB_OSC_TRANSMIT_SETUP(void);
void usb_osc_read_input(void);
void usb_osc_handle_input(OSCMessage &msg);
void usb_osc_transmit(void);

#endif

#endif /*__USB_OSC_TRANSMIT_H__*/