/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_MIDI_TRANSMIT_H__
#define __USB_MIDI_TRANSMIT_H__

#if USB_MIDI

#include <Encoder.h>                   // https://github.com/PaulStoffregen/Encoder
#include <MIDI.h>            // http://www.pjrc.com/teensy/td_midi.html

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"
#include "notes.h"

typedef struct preset preset_t; // Forward declaration
typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

extern Encoder encoder;
extern preset_t presets[];

void USB_MIDI_SETUP(void);
void usb_midi_handle_input(void);
void usb_control_change(byte channel, byte control, byte value);
void usb_midi_send_raw(image_t*interpFrame_ptr);
void usb_midi_send_interp(image_t*interpFrame_ptr);
void usb_midi_learn(llist_t* llist_ptr, preset_t* preset_ptr);
void usb_midi_send_blobs(llist_t* blobs_ptr);

#endif

#endif /*__USB_MIDI_TRANSMIT_H__*/
