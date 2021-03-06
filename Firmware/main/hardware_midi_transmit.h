/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/


#ifndef __HARDWARE_MIDI_TRANSMIT_H__
#define __HARDWARE_MIDI_TRANSMIT_H__

#include <MIDI.h>               // http://www.pjrc.com/teensy/td_midi.html

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"
#include "notes.h"
#include "mapping.h"

typedef struct preset preset_t;      // Forward declaration
typedef struct llist llist_t;        // Forward declaration
typedef struct blob blob_t;          // Forward declaration
typedef struct midiNode midiNode_t;  // Forward declaration
typedef struct ccPesets ccPesets_t;  // Forward declaration

extern preset_t presets[];
extern llist_t blobs;
extern llist_t midiIn;
extern llist_t midiOut;

void HARDWARE_MIDI_SETUP(void);
void hardware_midi_llist_init(llist_t* nodes_ptr, midiNode_t nodeArray_ptr, const int nodes); // TODO: Separation of concerns (SoC)
void hardware_midi_handle_input(void);
void hardware_midi_send_blobs(void);

#endif /*___HARDWARE_MIDI_TRANSMIT_H___*/
