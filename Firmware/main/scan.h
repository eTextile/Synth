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

typedef struct image image_t;       // Forward declaration
typedef struct preset preset_t;     // Forward declaration

#include <SPI.h>                    // https://github.com/PaulStoffregen/SPI
//#include "../library/SPI/SPI.h"   // https://github.com/PaulStoffregen/SPI

#include <ADC.h>                    // https://github.com/pedvide/ADC
//#include "../library/ADC/ADC.h"   // https://github.com/pedvide/ADC

void SETUP_SPI(void);

void SETUP_ADC(ADC* adc_ptr);

void calibrate_matrix(
  uint8_t* currentMode,
  uint8_t* lastMode,
  ADC* adc_ptr,
  ADC::Sync_result* result_ptr,
  uint8_t* offsetArray_ptr,
  uint8_t* shiftOutArray_ptr
);

void scan_matrix(
  ADC* adc_ptr,
  ADC::Sync_result* result_ptr,
  uint8_t* array_ptr,
  uint8_t* offsetArray_ptr,
  uint8_t* shiftOutArray_ptr
);

void print_adc(image_t* frame_ptr);

#endif /*__SCAN_H__*/
