/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MIDI_SEQ_H__
#define __MIDI_SEQ_H__

#include "config.h"

#define TAP_MAX_TAPS    8     // number of taps averaged to compute BPM
#define TAP_WINDOW_MS   3000  // gap > this (ms) resets the tap history
#define MIDI_PPQN       24    // MIDI clock pulses per quarter note
#define DEFAULT_BPM     120

void tap_tempo_hit(void);        // Record a tap and recompute BPM
void tap_tempo_clock_tick(void); // Send MIDI TimingClock pulses — call once per frame

void step_sequencer(void); // TODO
void arpeggiator(void);    // TODO

#endif /*__MIDI_SEQ_H__*/
