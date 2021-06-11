/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_SLIP_OSC_TRANSMIT_H__
#define __USB_SLIP_OSC_TRANSMIT_H__

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"

#include <OSCBoards.h>              // https://github.com/CNMAT/OSC
#include <OSCMessage.h>             // https://github.com/CNMAT/OSC
#include <OSCBundle.h>              // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h>   // https://github.com/CNMAT/OSC

typedef struct image image_t;       // Forward declaration
typedef struct llist llist_t;       // Forward declaration
typedef struct blob blob_t;         // Forward declaration
typedef struct preset preset_t;     // Forward declaration

extern Encoder encoder;
extern image_t rawFrame;
extern image_t interpFrame;
extern llist_t blobs;
extern preset_t presets[];
extern uint8_t currentMode;
extern uint8_t lastMode;

void USB_SLIP_OSC_SETUP(void);
void usb_slip_osc_handle_input(void);

#endif /*__USB_OSC_TRANSMIT_H__*/
