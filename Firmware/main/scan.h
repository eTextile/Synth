/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __SCAN_H__
#define __SCAN_H__

#include "config.h"
#include "presets.h"
#include "blob.h"

typedef struct image image_t;   // Forward declaration
extern image_t rawFrame;        // Exposed local declaration see scan.cpp

#include <SPI.h>                // https://github.com/PaulStoffregen/SPI
#include <ADC.h>                // https://github.com/pedvide/ADC

void SPI_SETUP(void);
void ADC_SETUP(void);
void SCAN_SETUP(void);
void calibrate_matrix(void);
void scan_matrix(void);

#endif /*__SCAN_H__*/
