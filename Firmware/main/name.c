/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#if MIDI_USB

#include "usb_names.h"

//#define MIDI_NAME {'E','T','E','X','T','I','L','E','-','S','Y','N','T','H'}
//#define MIDI_NAME_LEN  14

#define MIDI_NAME      {'E','2','5','6'}
#define MIDI_NAME_LEN  4

struct usb_string_descriptor_struct usb_string_product_name = {
  2 + MIDI_NAME_LEN * 2,
  3,
  MIDI_NAME
};

#endif
