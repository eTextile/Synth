/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.

  TODO:
    -> https://github.com/pedvide/ADC/issues/76
      -> https://github.com/KurtE/DmaSpi
*/

#include "scan.h"
#include "blob.h"

#include <SPI.h> // https://github.com/PaulStoffregen/SPI
#include <ADC.h> // https://github.com/pedvide/ADC

// Static allocation — avoids heap fragmentation and pointer indirection overhead.
ADC adc;
ADC::Sync_result result;

#define SS1_PIN   0  // Hardware SPI1 (SELECT : STCP)
#define SCK1_PIN  27 // Hardware SPI1 (CLOCK  : SHCP)
#define MOSI1_PIN 26 // Hardware SPI1 (DATA   : DS)

#define SET_ORIGIN_X // X-axis origin positioning
#define SET_ORIGIN_Y // Y-axis origin positioning

#define ADC0_PIN A3  // Pin 17 — output of multiplexer A (SIG pin)
#define ADC1_PIN A2  // Pin 16 — output of multiplexer B (SIG pin)

#define DISCHARGE_DEL 10 // µs — capacitance discharge time before ADC read

#define DUAL_COLS (RAW_COLS / 2)

#define CALIBRATION_CYCLES 10

uint8_t raw_frame_array[RAW_FRAME]    = {0}; // ADC values after offset subtraction
uint8_t offset_frame_array[RAW_FRAME] = {0}; // Baseline (no-touch) ADC values

image_t raw_frame;    // image_t view over raw_frame_array
image_t offset_frame; // image_t view over offset_frame_array

// One byte per dual-column: |ENA|A|B|C|ENA|A|B|C| — configures the two 8:1 analog muxes.
uint8_t set_dual_cols[DUAL_COLS] = {
#if defined(SET_ORIGIN_X)
  0x33, 0x00, 0x11, 0x22, 0x44, 0x66, 0x77, 0x55
#else
  0x55, 0x77, 0x66, 0x44, 0x22, 0x11, 0x00, 0x33
#endif
};

#undef SPI_HAS_TRANSACTION
#define SPI_HAS_TRANSACTION 0
// will look at https://github.com/RobTillaart/FastShiftInOut

// Selects the active row + dual-column via the three shift registers.
// TODO: SPI1.transfer16(bswap(set_rows)) + SPI1.transfer(col) to halve SPI overhead.
static inline void spi_select_cell(uint16_t set_rows, uint8_t cols) {
  digitalWriteFast(SS1_PIN, LOW);
  SPI1.transfer((uint8_t)(set_rows & 0xFF));         // Low byte  → first shift register
  SPI1.transfer((uint8_t)((set_rows >> 8) & 0xFF));  // High byte → second shift register
  SPI1.transfer(set_dual_cols[cols]);                // Mux config → third shift register
  digitalWriteFast(SS1_PIN, HIGH);
}

// Discharges both ADC input capacitances to GND, then releases to INPUT for sampling.
// digitalWriteFast(LOW) pre-sets the data register before pinMode(OUTPUT)
// so the pin drives LOW the instant direction changes — no extra digitalWrite needed.
static inline void discharge_and_sample(void) {
  digitalWriteFast(ADC0_PIN, LOW);
  digitalWriteFast(ADC1_PIN, LOW);
  pinMode(ADC0_PIN, OUTPUT);
  pinMode(ADC1_PIN, OUTPUT);
  delayMicroseconds(DISCHARGE_DEL);
  pinMode(ADC0_PIN, INPUT);
  pinMode(ADC1_PIN, INPUT);
#if defined(SET_ORIGIN_X)
  result = adc.analogSynchronizedRead(ADC0_PIN, ADC1_PIN);
#else
  result = adc.analogSynchronizedRead(ADC1_PIN, ADC0_PIN);
#endif
}

static inline void setup_spi(void) {
  pinMode(SS1_PIN, OUTPUT);
  SPI1.begin();
  SPI1.beginTransaction(SPISettings(30000000, MSBFIRST, SPI_MODE0)); // 74HC595BQ max: 100 MHz
  digitalWriteFast(SS1_PIN, LOW);
  digitalWriteFast(SS1_PIN, HIGH);
}

static inline void setup_adc(void) {
  pinMode(ADC0_PIN, INPUT);
  pinMode(ADC1_PIN, INPUT);
  adc.adc0->setAveraging(1);
  adc.adc0->setResolution(8);
  adc.adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc.adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);
  adc.adc1->setAveraging(1);
  adc.adc1->setResolution(8);
  adc.adc1->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc.adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);
}

void scan_setup(void) {
  setup_spi();
  setup_adc();

  raw_frame.data_ptr = &raw_frame_array[0];
  raw_frame.num_cols = RAW_COLS;
  raw_frame.num_rows = RAW_ROWS;

  offset_frame.data_ptr = &offset_frame_array[0];
  offset_frame.num_cols = RAW_COLS;
  offset_frame.num_rows = RAW_ROWS;
}

// Builds the baseline offset frame over CALIBRATION_CYCLES passes.
// Columns are read two at a time (dual-column mux); rows are driven one at a time.
void matrix_calibrate(void) {
  memset(&offset_frame_array[0], 0, RAW_FRAME);

  for (uint8_t i = 0; i < CALIBRATION_CYCLES; i++) {
    for (uint8_t cols = 0; cols < DUAL_COLS; cols++) {
#if defined(SET_ORIGIN_Y)
      uint16_t set_rows = 0x0001;
#else
      uint16_t set_rows = 0x8000;
#endif
      for (uint8_t row = 0; row < RAW_ROWS; row++) {
        spi_select_cell(set_rows, cols);

        uint8_t index_a = row * RAW_COLS + cols;
        uint8_t index_b = index_a + DUAL_COLS;

        discharge_and_sample();

        offset_frame_array[index_a] = max(offset_frame_array[index_a], (uint8_t)result.result_adc0);
        offset_frame_array[index_b] = max(offset_frame_array[index_b], (uint8_t)result.result_adc1);

#if defined(SET_ORIGIN_Y)
        set_rows <<= 1;
#else
        set_rows >>= 1;
#endif
      }
    }
  }
}

// Scans the full 16×16 matrix and stores offset-subtracted values in raw_frame_array.
// Columns are read two at a time (dual-column mux); rows are driven one at a time.
void matrix_scan(void) {
  for (uint8_t cols = 0; cols < DUAL_COLS; cols++) {
#if defined(SET_ORIGIN_Y)
    uint16_t set_rows = 0x0001;
#else
    uint16_t set_rows = 0x8000;
#endif
    for (uint8_t row = 0; row < RAW_ROWS; row++) {
      spi_select_cell(set_rows, cols);

      uint8_t index_a = row * RAW_COLS + cols;
      uint8_t index_b = index_a + DUAL_COLS;

      discharge_and_sample();

      uint8_t valA = (uint8_t)result.result_adc0;
      raw_frame_array[index_a] = (valA > offset_frame_array[index_a])
        ? min(127, valA - offset_frame_array[index_a])
        : 0;

      uint8_t valB = (uint8_t)result.result_adc1;
      raw_frame_array[index_b] = (valB > offset_frame_array[index_b])
        ? min(127, valB - offset_frame_array[index_b])
        : 0;

#if defined(SET_ORIGIN_Y)
      set_rows <<= 1;
#else
      set_rows >>= 1;
#endif
    }
  }

#if defined(USB_MIDI_SERIAL) && defined(DEBUG_ADC)
  for (uint8_t posY = 0; posY < RAW_ROWS; posY++) {
    uint8_t* row_ptr = COMPUTE_IMAGE_ROW_PTR(&raw_frame, posY);
    for (uint8_t posX = 0; posX < RAW_COLS; posX++) {
      Serial.printf("_%d", IMAGE_GET_PIXEL_FAST(row_ptr, posX));
    }
    Serial.printf("\n");
  }
  Serial.printf("\n");
#endif
}
