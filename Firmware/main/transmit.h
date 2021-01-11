/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#include <MIDI.h>                   // http://www.pjrc.com/teensy/td_midi.html
/*
#include <OSCBoards.h>              // https://github.com/CNMAT/OSC
#include <OSCMessage.h>             // https://github.com/CNMAT/OSC
#include <OSCBundle.h>              // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h>   // https://github.com/CNMAT/OSC
*/

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"

//SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);

typedef struct preset preset_t; // Forward declaration
typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

void TRANSMIT_SLIP_OSC_SETUP(void);
void transmit_blobs_slipOsc(llist_t* blobList_ptr, preset_t* preset_ptr);

void USB_MIDI_SETUP(void);
void transmit_blobs_midi(llist_t* blobList_ptr, preset_t* preset_ptr);

#endif /*__TRANSMIT_H__*/
