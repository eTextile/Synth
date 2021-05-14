/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __SEQ_H__
#define __SEQ_H__

#include "seq.h"
#include "llist.h"
#include "blob.h"
#include "mapping.h"

typedef struct llist llist_t;       // Forward declaration
typedef struct blob blob_t;         // Forward declaration
typedef struct midiNode midiNode_t; // Forward declaration
typedef struct tSwitch tSwitch_t;   // Forward declaration
typedef struct vSlider vSlider_t;   // Forward declaration
typedef struct hSlider hSlider_t;   // Forward declaration
typedef struct cSlider cSlider_t;   // Forward declaration
//typedef struct grid grid_t;         // Forward declaration

typedef struct seq seq_t;
struct seq {
  uint16_t timeInterval[];
  uint8_t* seqframe;
};

void tapTempo(tSwitch_t* tSwitch_ptr, seq_t* seq_ptr); // TODO
void stepSequencer(tSwitch_t* tSwitch_ptr, seq_t* seq_ptr); // TODO
void arpeggiator(llist_t* llist_ptr); // TODO

#endif /*__SEQ_H__*/
