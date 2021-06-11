/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_MIDI_TRANSMIT_H__
#define __USB_MIDI_TRANSMIT_H__

#include <Encoder.h>            // https://github.com/PaulStoffregen/Encoder
#include <MIDI.h>               // http://www.pjrc.com/teensy/td_midi.html
#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"
#include "notes.h"

typedef struct preset preset_t; // Forward declaration
typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

extern Encoder encoder;
extern image_t rawFrame;
extern image_t interpFrame;
extern llist_t blobs;
extern preset_t presets[];

void USB_MIDI_SETUP(void);
void usb_midi_llist_init(llist_t* nodes_ptr, midiNode_t* nodeArray_ptr, const int nodes); // TODO: Separation of concerns (SoC)
void usb_midi_handle_input(void);
void usb_midi_update_presets(byte channel, byte control, byte value);
void usb_midi_send_raw(void);
void usb_midi_send_interp(void);
void usb_midi_learn(void);
void usb_midi_send_blobs(void);

#endif /*__USB_MIDI_TRANSMIT_H__*/
