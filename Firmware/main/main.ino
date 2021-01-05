/*
  ** eTextile-Synthetizer - Firmware v1.0 **
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "hardware.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"
#include "audio.h"

uint8_t offsetArray[RAW_FRAME] = {0};             // 1D Array to store smallest values
uint8_t frameArray[RAW_FRAME] = {0};              // 1D Array to store ofseted analog input values

uint8_t bitmapArray[NEW_FRAME] = {0};             // 1D Array to store binary values (16*16) array containing (64*64) values
uint8_t interpFrameArray[NEW_FRAME] = {0};        // 1D Array to store bilinear interpolated values

xylr_t lifoArray[LIFO_MAX_NODES] = {0};           // 1D Array to store lifo nodes
blob_t blobArray[MAX_NODES] = {0};                // 1D Array to store all blobs

uint8_t blobPacket[BLOB_PACKET_SIZE] = {0};       // 1D Array to store blob values

image_t  inputFrame;            // Input frame values
interp_t interp;                //
image_t  interpolatedFrame;     //
image_t  bitmap;                // Used by Scanline Flood Fill algorithm / SFF
lifo_t   lifo_stack;            // Lifo free nodes stack
lifo_t   lifo;                  // Lifo stack
llist_t  blobs_stack;           // Blobs free nodes linked list
llist_t  blobs;                 // Intermediate blobs linked list
llist_t  outputBlobs;           // Output blobs linked list

Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

Button BUTTON_L = Button();
Button BUTTON_R = Button();

AudioControlSGTL5000 sgtl5000;

ADC* adc = new ADC();           // ADC object
ADC::Sync_result result;        // Store ADC_0 & ADC_1

// Array to store all parameters used to configure the two 8:1 analog multiplexeurs
// Each byte |ENA|A|B|C|ENA|A|B|C|
uint8_t setDualRows[DUAL_ROWS] = {
#if SET_ORIGIN_Y == 1
  0x33, 0x00, 0x11, 0x22, 0x44, 0x66, 0x77, 0x55
#else
  0x55, 0x77, 0x66, 0x44, 0x22, 0x11, 0x00, 0x33
#endif /*__SET_ORIGIN_Y__*/
};

uint8_t lastMode = CALIBRATE;    // Initialise lastMode with the DEFAULT_MODE
uint8_t currentMode = LINE_OUT;  // Initialise currentMode with the DEFAULT_MODE
uint8_t blobValSelector = 0;     // Used for MIDI_LEARN mode
uint8_t zThreshold = 10;         //

#if DEBUG_FPS
elapsedMillis curentMillisFps;
unsigned int fps = 0;
#endif

#if DEBUG_ADC || DEBUG_BITMAP || DEBUG_INTERP
elapsedMillis timerDebug;
#endif

boolean toggleSwitch = false;
boolean doLoadPreset = true;
boolean doSavePreset = false;
boolean calibrateMatrix = true;

elapsedMillis ledTimer;
uint8_t ledIterations = 0;

preset_t presets[7] = {
  {0, 13, 31, 21, 21, true, LOW, LOW },   // LINE_OUT
  {1, 0, 15, 5, 5, true, HIGH, LOW },     // SIG_IN
  {2, 0, 31, 17, 17, true, LOW, HIGH },   // SIG_OUT / min 13
  {3, 0, 50, 8, 8, true, HIGH, HIGH },    // THRESHOLD
  {4, 0, 6, 0, 0, true, NULL, NULL },     // MIDI_LEARN [ID, alive, X, Y, W, H, D]
  {5, 0, 0, 0, 0, true, NULL, NULL },     // CALIBRATE
  {6, 0, 0, 0, 0, true, NULL, NULL }      // SAVE
};

AudioSynthWaveform       waveform_A;
AudioSynthWaveform       waveform_B;
AudioOutputI2S           i2s1;
AudioOutputAnalogStereo  dacs1;
AudioConnection          patchCord1(waveform_A, 0, i2s1, 0);
AudioConnection          patchCord2(waveform_A, 0, dacs1, 0);
AudioConnection          patchCord3(waveform_B, 0, i2s1, 1);
AudioConnection          patchCord4(waveform_B, 0, dacs1, 1);

void setup() {

#if DEBUG_ADC || DEBUG_INTERP || DEBUG_BLOBS || DEBUG_SFF_BITMAP || DEBUG_FPS
  Serial.begin(BAUD_RATE);       // Start Serial communication using 230400 baud
  //while (!Serial.dtr());         // Wait for user to start the serial monitor
#endif

  SETUP_LEDS();
  SETUP_SWITCHES(&BUTTON_L, &BUTTON_R);
  SETUP_SPI();
  SETUP_ADC(adc);
  //SETUP_DAC(&sgtl5000, &presets[0], &waveform_A, &waveform_B);

  SETUP_INTERP(
    &inputFrame,          // image_t*
    &frameArray[0],       // uint8_t*
    &interpolatedFrame,   // image_t*
    &interpFrameArray[0], // uint8_t*
    &interp               // interp_t*
  );
  
  SETUP_BLOB(
    &inputFrame,          // image_t*
    &bitmap,              // image_t*
    &bitmapArray[0],      // uint8_t*
    &lifo,                // lifo_t*
    &lifo_stack,          // lifo_t*
    &lifoArray[0],        // xylr_t*
    &blobs,               // list_t*
    &blobs_stack,         // list_t*
    &blobArray[0],        // blob_t*
    &outputBlobs          // list_t*
  );
}

//////////////////// LOOP
void loop() {

  if (doLoadPreset) e256_preset_load(&presets[0], &doLoadPreset);
  if (doSavePreset) e256_preset_save(&presets[0], &doSavePreset);

  e256_update_buttons(
    &BUTTON_L,
    &BUTTON_R,
    &encoder,
    &presets[0],
    &currentMode,
    &lastMode,
    &ledIterations,
    &toggleSwitch,
    &ledTimer
  );

  e256_update_preset(
    &encoder,
    &presets[currentMode],
    &zThreshold,
    &blobValSelector,
    &calibrateMatrix,
    &doSavePreset,
    &sgtl5000,
    &ledTimer
  );

  e256_update_leds(
    &presets[currentMode],
    &ledTimer,
    &currentMode,
    &lastMode
  );

  if (calibrateMatrix) {
    calibrateMatrix = false;
    e256_calibrate_matrix(
      adc,
      &result,
      &offsetArray[0],
      &setDualRows[0]
    );
  }

  e256_scan_matrix(
    adc,
    &result,
    &frameArray[0],
    &offsetArray[0],
    &setDualRows[0]
  );

#if DEBUG_ADC
  if (timerDebug >= 100) {
    timerDebug = 0;
    e256_print_adc(&inputFrame);
  }
#endif

  e256_interp_matrix(&interpolatedFrame, &inputFrame, &interp);

#if DEBUG_INTERP
  if (timerDebug >= 100) {
    timerDebug = 0;
    e256_print_interp(&interpolatedFrame);
  }
#endif

  e256_find_blobs(
    zThreshold,         // uint8_t zThreshold
    &interpolatedFrame, // image_t (uint8_t array[NEW_FRAME] - 64*64 1D array)
    &bitmap,            // image_t (uint8_t array[NEW_FRAME] - 64*64 1D array)
    &lifo_stack,        // lifo_t
    &lifo,              // lifo_t
    &blobs_stack,       // list_t
    &blobs,             // list_t
    &outputBlobs        // list_t
  );

#if DEBUG_BLOBS
  e256_print_blobs(&outputBlobs);
#endif

#if DEBUG_BITMAP
  if (timerDebug >= 1000) {
    timerDebug = 0;
    e256_print_bitmap(&bitmap);
  }
#endif

  e256_make_noise(&outputBlobs, &sgtl5000, &waveform_A, &waveform_B);

#if DEBUG_FPS
  if (curentMillisFps >= 1000) {
    curentMillisFps = 0;
    Serial.println(fps);
    fps = 0;
  }
  fps++;
#endif
}
