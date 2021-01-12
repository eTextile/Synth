/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#if USB_MIDI
#include <MIDI.h>                   // http://www.pjrc.com/teensy/td_midi.html
#endif

#if USB_SLIP_OSC
#include <OSCBoards.h>              // https://github.com/CNMAT/OSC
#include <OSCMessage.h>             // https://github.com/CNMAT/OSC
#include <OSCBundle.h>              // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h>   // https://github.com/CNMAT/OSC
//SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB); // FIXME
#endif

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"

typedef struct preset preset_t; // Forward declaration
typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

#if USB_SLIP_OSC
void USB_SLIP_OSC_SETUP(void);
void blob_usb_slipOsc(blob_t* blob_ptr, preset_t* preset_ptr, OSCBundle* synthOSC);
#endif

#if USB_MIDI
void USB_MIDI_SETUP(void);
void blob_usb_midi_learn(llist_t* blobs_ptr, preset_t* preset_ptr);
void blob_usb_midi_play(llist_t* blobs_ptr);
#endif

#endif /*__TRANSMIT_H__*/
