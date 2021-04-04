/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"

#if USB_MIDI || HARDWARE_MIDI
#include <MIDI.h>                   // http://www.pjrc.com/teensy/td_midi.html
#endif

#if USB_SLIP_OSC
#include <OSCBoards.h>              // https://github.com/CNMAT/OSC
#include <OSCMessage.h>             // https://github.com/CNMAT/OSC
#include <OSCBundle.h>              // https://github.com/CNMAT/OSC
#include <SLIPEncodedUSBSerial.h>   // https://github.com/CNMAT/OSC
#endif

typedef struct preset preset_t; // Forward declaration
typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

typedef struct midiNode midiNode_t;
struct midiNode {
  lnode_t node;
  uint8_t pithch;
  uint8_t velocity;
  //uint8_t channel;
};

typedef struct ccPesets ccPesets_t;
struct ccPesets {
  uint8_t blobID;
  uint8_t mappVal;
  int8_t cChange;
  int8_t midiChannel;
  int8_t val;
};

#if HARDWARE_MIDI
void HARDWARE_MIDI_SETUP(void);
void midiIn_llist_init(llist_t *list_ptr, midiNode_t* nodesArray, uint8_t max_nodes);
void handleMidiInput(llist_t* llist_ptr, llist_t* nodeStack_ptr); // TODO
void sendControlChange_b(llist_t* llist_ptr, ccPesets_t* ccPesets_ptr);
#endif

#if USB_SLIP_OSC
void USB_SLIP_OSC_SETUP(void);
void blobs_usb_slipOsc(llist_t* llist_ptr);
#endif

#if USB_MIDI
void USB_MIDI_SETUP(void);
void blobs_usb_midi_learn(llist_t* llist_ptr, preset_t* preset_ptr);
void blobs_usb_midi_play(llist_t* llist_ptr);
#endif

#endif /*__TRANSMIT_H__*/
