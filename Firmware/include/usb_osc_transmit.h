/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_OSC_TRANSMIT_H__
#define __USB_OSC_TRANSMIT_H__

#if defined(USB_OSC)

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"
#include "midi_nodes.h"

#include <OSCBoards.h>              // https://github.com/CNMAT/OSC
#include <OSCMessage.h>             // https://github.com/CNMAT/OSC
#include <OSCBundle.h>              // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h>   // https://github.com/CNMAT/OSC

void USB_OSC_TRANSMIT_SETUP(void);
void usb_osc_read_input(void);
void usb_osc_handle_input(OSCMessage &msg);
void usb_osc_transmit(void);

#endif

#endif /*__USB_OSC_TRANSMIT_H__*/