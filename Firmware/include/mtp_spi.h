/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __MTP_SPI_H__
#define __MTP_SPI_H__

#include <LittleFS.h>
#include <MTP_Teensy.h>

#include "config.h"
#include "json_parser.h"

void MTP_SPI_SETUP(void);
void handle_mtp_spi(void);

void write_start(void);
void write_stop(void);
void dump_json(void);

#endif /*__MTP_SPI_H__*/