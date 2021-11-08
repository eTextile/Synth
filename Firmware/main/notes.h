/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __NOTES_H__
#define __NOTES_H__

#include "config.h"

// See: https://www.c-thru-music.com/cgi/?page=layout_octaves
#if GRID_LAYOUT_HARMONIC
#define GRID_COLS         14
#define GRID_ROWS         9
#define GRID_KEYS         (GRID_COLS * GRID_ROWS)
#endif

#if GRID_LAYOUT_DEFAULT
#define GRID_COLS        12
#define GRID_ROWS        11
#define GRID_KEYS        (GRID_COLS * GRID_ROWS)
#endif

#if GRID_LAYOUT_MIDI_IN
#define GRID_COLS        8
#define GRID_ROWS        8
#define GRID_KEYS        (GRID_COLS * GRID_ROWS)
#endif

extern uint8_t gridLayout[];

#endif /*__NOTES_H__*/
