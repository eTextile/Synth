/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __SERIAL_TRANSMIT_H__
#define __SERIAL_TRANSMIT_H__

#include "config.h"
#include "llist.h"
#include "blob.h"
#include "presets.h"
#include "midi_transmit.h"

void SERIAL_TRANSMIT_SETUP(void);
void read_serial_input(void);
void serial_transmit(void);

#endif /*__SERIAL_TRANSMIT_H__*/
