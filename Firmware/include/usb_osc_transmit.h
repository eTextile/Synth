/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __USB_OSC_TRANSMIT_H__
#define __USB_OSC_TRANSMIT_H__

#include "config.h"
#include "midi_bus.h"

inline void osc_getInput(uint8_t status, uint8_t note, uint8_t value){
  midiNode_t* node_ptr = (midiNode_t*)llist_pop_front(&midi_node_stack);
  node_ptr->midiMsg.status = status;
  node_ptr->midiMsg.data1 = note;
  node_ptr->midiMsg.data2 = value;
  llist_push_front(&midiIn, node_ptr); 
};

void USB_OSC_TRANSMIT_SETUP(void);
void usb_osc_read_input(void);
void usb_osc_transmit(void);

#endif /*__USB_OSC_TRANSMIT_H__*/