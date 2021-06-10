/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __HARDWARE_MIDI_TRANSMIT_H__
#define __HARDWARE_MIDI_TRANSMIT_H__

//#if HARDWARE_MIDI

#include <MIDI.h>               // http://www.pjrc.com/teensy/td_midi.html

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"
#include "notes.h"

typedef struct preset preset_t; // Forward declaration
typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

extern MIDI_NAMESPACE::SerialMIDI<HardwareSerial>serialMIDI;
extern MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> MIDI;

void HARDWARE_MIDI_SETUP(void);
void midi_llist_init(llist_t* midiNodes_ptr, midiNode_t* nodeArray_ptr);
boolean midi_handle_hardware_input(llist_t* llist_ptr);
void control_change(llist_t* llist_ptr, ccPesets_t* ccPesets_ptr);

//#endif

#endif /*___HARDWARE_MIDI_TRANSMIT_H___*/
