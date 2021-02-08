/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "scan.h"

void SETUP_SPI(void) {

  pinMode(SS1_PIN, OUTPUT);                                               // Set the Slave Select Pin as OUTPUT
  SPI1.begin();                                                           // Start the SPI module
  SPI1.beginTransaction(SPISettings(30000000, MSBFIRST, SPI_MODE0));      // 74HC595BQ Shift out register frequency is 100 MHz = 100000000 Hz
  digitalWrite(SS1_PIN, LOW);                                             // Set latchPin LOW
  digitalWrite(SS1_PIN, HIGH);                                            // Set latchPin HIGH
}

void SETUP_ADC(ADC *adc) {

  pinMode(ADC0_PIN, INPUT);                                               // PIN A2 (Teensy 4.0 pin 16)
  pinMode(ADC1_PIN, INPUT);                                               // PIN A3 (Teensy 4.0 pin 17)

  adc->adc0->setAveraging(1);                                             // Set number of averages
  adc->adc0->setResolution(8);                                            // Set bits of resolution
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);   // Change the conversion speed
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);       // Change the sampling speed
  //adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);      // Change the conversion speed
  //adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);          // Change the sampling speed

  adc->adc1->setAveraging(1);                                             // Set number of averages
  adc->adc1->setResolution(8);                                            // Set bits of resolution
  adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);   // Change the conversion speed
  adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);       // Change the sampling speed
  //adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);      // Change the conversion speed/*
  //adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);          // Change the sampling speed
}

// Columns are analog INPUT_PINS reded two by two
// Rows are digital OUTPUT_PINS supplyed one by one sequentially with 3.3V
void calibrate_matrix(ADC* adc_ptr, ADC::Sync_result* result_ptr, uint8_t* ofsetArray_ptr, uint8_t* shiftOutArray_ptr) {

  uint16_t setRows;

  for (uint8_t i = 0; i < CALIBRATION_CYCLES; i++) {

    for (uint8_t col = 0; col < DUAL_COLS; col++) {         // ANNALOG_PINS [0-7] with [8-15]
#if SET_ORIGIN_Y
      setRows = 0x1;                                        // Reset to [0000 0000 0000 0001]
#else
      setRows = 0x8000;                                     // Reset to [1000 0000 0000 0000]
#endif
      for (uint8_t row = 0; row < RAW_ROWS; row++) {        // DIGITAL_PINS [0-15]
        digitalWrite(SS1_PIN, LOW);                         // Set the Slave Select Pin LOW
        //SPI1.transfer16(setRows);                         // Set up the two OUTPUT shift registers (FIXME)
        SPI1.transfer((uint8_t)(setRows & 0xFF));           // Shift out one byte to setup one OUTPUT shift register
        SPI1.transfer((uint8_t)((setRows >> 8) & 0xFF));    // Shift out one byte to setup one OUTPUT shift register
        SPI1.transfer(shiftOutArray_ptr[col]);              // Shift out one byte that setup the two INPUT 8:1 analog multiplexers
        digitalWrite(SS1_PIN, HIGH);                        // Set the Slave Select Pin HIGH
        uint8_t indexA = row * RAW_COLS + col;              // Compute 1D array indexA
        uint8_t indexB = indexA + DUAL_COLS;                // Compute 1D array indexB
        delayMicroseconds(15);
        *result_ptr = adc_ptr->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);
        uint8_t ADC0_val = result_ptr->result_adc0;
        if (ADC0_val > ofsetArray_ptr[indexA]) ofsetArray_ptr[indexA] = ADC0_val;
        uint8_t ADC1_val = result_ptr->result_adc1;
        if (ADC1_val > ofsetArray_ptr[indexB]) ofsetArray_ptr[indexB] = ADC1_val;
#if SET_ORIGIN_Y
        setRows = setRows << 1;
#else
        setRows = setRows >> 1;
#endif
      }
    }
  }
}

// Columns are analog INPUT_PINS reded two by two
// Rows are digital OUTPUT_PINS supplyed one by one sequentially with 3.3V
void scan_matrix(ADC* adc_ptr, ADC::Sync_result* result_ptr, uint8_t* array_ptr, uint8_t* offsetArray_ptr, uint8_t* shiftOutArray_ptr) {

  uint16_t setRows;

  for (uint8_t col = 0; col < DUAL_COLS; col++) {         // ANNALOG_PINS [0-7] with [8-15]
#if SET_ORIGIN_Y
    setRows = 0x1;                                        // Reset to [0000 0000 0000 0001]
#else
    setRows = 0x8000;                                     // Reset to [1000 0000 0000 0000]
#endif
    for (uint8_t row = 0; row < RAW_ROWS; row++) {        // DIGITAL_PINS [0-15]
      digitalWrite(SS1_PIN, LOW);                         // Set the Slave Select Pin LOW
      //SPI1.transfer16(setRows);                         // Set up the two OUTPUT shift registers (FIXME)
      SPI1.transfer((uint8_t)(setRows & 0xFF));           // Shift out LSB byte to setup one OUTPUT shift register
      SPI1.transfer((uint8_t)((setRows >> 8) & 0xFF));    // Shift out MSB byte to setup one OUTPUT shift register
      SPI1.transfer(shiftOutArray_ptr[col]);              // Shift out one byte that setup the two INPUT 8:1 analog multiplexers
      digitalWrite(SS1_PIN, HIGH);                        // Set the Slave Select Pin HIGH
      uint8_t indexA = row * RAW_COLS + col;              // Compute 1D array indexA
      uint8_t indexB = indexA + DUAL_COLS;                // Compute 1D array indexB
      delayMicroseconds(15);
      *result_ptr = adc_ptr->analogSynchronizedRead(ADC0_PIN, ADC1_PIN);
      uint8_t valA = result_ptr->result_adc0;
      valA > offsetArray_ptr[indexA] ? array_ptr[indexA] = valA - offsetArray_ptr[indexA] : array_ptr[indexA] = 0;
      uint8_t valB = result_ptr->result_adc1;
      valB > offsetArray_ptr[indexB] ? array_ptr[indexB] = valB - offsetArray_ptr[indexB] : array_ptr[indexB] = 0;
#if SET_ORIGIN_Y
      setRows = setRows << 1;
#else
      setRows = setRows >> 1;
#endif
    }
  }
}

void print_adc(image_t* image_ptr) {
  for (uint8_t posY = 0; posY < image_ptr->numRows; posY++) {
    uint8_t* row_ptr = COMPUTE_IMAGE_ROW_PTR(image_ptr, posY);
    for (uint8_t posX = 0; posX < image_ptr->numCols; posX++) {
      Serial.printf("\t%d", IMAGE_GET_PIXEL_FAST(row_ptr, posX));
    }
    Serial.printf("\n");
  }
  Serial.printf("\n");
}
