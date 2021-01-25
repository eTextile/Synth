/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"

#if MIDI_USB
#include <MIDI.h>                   // http://www.pjrc.com/teensy/td_midi.html
#endif

#if USB_SLIP_OSC
#include <OSCBoards.h>              // https://github.com/CNMAT/OSC
#include <OSCMessage.h>             // https://github.com/CNMAT/OSC
#include <OSCBundle.h>              // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h>   // https://github.com/CNMAT/OSC
SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB); // FIXME
#endif

typedef struct preset preset_t; // Forward declaration
typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

#if SLIP_OSC
void SETUP_SLIP_OSC(void);
void blobs_usb_slipOsc(llist_t* blobs_ptr);
#endif

#if MIDI_USB
void SETUP_MIDI_USB(void);
void blobs_usb_midi_learn(llist_t* blobs_ptr, preset_t* preset_ptr);
void blobs_usb_midi_play(llist_t* blobs_ptr);
#endif

#endif /*__TRANSMIT_H__*/
