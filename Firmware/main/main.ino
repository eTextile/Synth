/*
  ** eTextile-Synthetizer - Firmware v1.0 **
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "presets.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"
#include "mapping.h"
#include "audio.h"
#include "transmit.h"

// Array to store all parameters used to configure the two 8:1 analog multiplexeurs
// Each byte |ENA|A|B|C|ENA|A|B|C|
uint8_t setDualRows[DUAL_COLS] = {
#if SET_ORIGIN_X
  0x33, 0x00, 0x11, 0x22, 0x44, 0x66, 0x77, 0x55
#else
  0x55, 0x77, 0x66, 0x44, 0x22, 0x11, 0x00, 0x33
#endif
};

uint8_t offsetArray[RAW_FRAME] = {0};             // 1D Array to store smallest values
uint8_t frameArray[RAW_FRAME] = {0};              // 1D Array to store ofseted analog input values

uint8_t bitmapArray[NEW_FRAME] = {0};             // 1D Array to store binary values (16*16) array containing (64*64) values
uint8_t interpFrameArray[NEW_FRAME] = {0};        // 1D Array to store bilinear interpolated values

xylr_t lifoArray[LIFO_MAX_NODES] = {0};           // 1D Array to store lifo nodes
blob_t blobArray[MAX_NODES] = {0};                // 1D Array to store all blobs

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

ADC* adc = new ADC();           // ADC object
ADC::Sync_result result;        // Store ADC_0 & ADC_1

AudioControlSGTL5000 sgtl5000;

AudioSynthWaveform       waveform1;
AudioSynthWaveformSineModulated sine_fm1;
AudioEffectFade          fade1;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(waveform1, sine_fm1);
AudioConnection          patchCord2(sine_fm1, fade1);
AudioConnection          patchCord3(fade1, 0, i2s1, 0);
AudioConnection          patchCord4(fade1, 0, i2s1, 1);


uint8_t lastMode = CALIBRATE;
uint8_t currentMode = LINE_OUT;  // Initialise currentMode with the DEFAULT_MODE

#if DEBUG_FPS
elapsedMillis curentMillisFps;
unsigned int fps = 0;
#endif

#if DEBUG_ADC || DEBUG_BITMAP || DEBUG_INTERP
elapsedMillis timerDebug;
#endif

boolean toggleSwitch = false;
boolean loadPreset = true;
boolean savePreset = false;
boolean calibrateMatrix = true;

elapsedMillis ledTimer;
uint8_t ledIterations = 0;

preset_t presets[7] = {
  {0, 13, 31, 21, 21, true, LOW, LOW },   // LINE_OUT
  {1, 0, 15, 5, 5, true, HIGH, LOW },     // SIG_IN
  {2, 0, 31, 17, 17, true, LOW, HIGH },   // SIG_OUT
  {3, 10, 60, 40, 40, true, HIGH, HIGH },  // THRESHOLD
  {4, 1, 6, 1, 1, true, NULL, NULL },     // MIDI_LEARN [ID, alive, X, Y, W, H, D]
  {5, 0, 0, 0, 0, true, NULL, NULL },     // CALIBRATE
  {6, 0, 0, 0, 0, true, NULL, NULL }      // SAVE
};

// Testing mapping fonctions
switch_t tapSwitch_A = {10, 10, 5, 1000, false}; // ARGS [posX, posY, rSize, debounceTimer, state]
switch_t tapSwitch_B = {40, 30, 5, 1000, false}; // ARGS [posX, posY, rSize, debounceTimer, state]

switch_t modeSwitch_A = {30, 10, 5, 1000, false};
switch_t modeSwitch_B = {40, 25, 5, 1000, false};

void setup() {

#if DEBUG_ADC || DEBUG_INTERP || DEBUG_BLOBS || DEBUG_SFF_BITMAP || DEBUG_FPS
  Serial.begin(BAUD_RATE); // Start Serial communication using 230400 baud
#endif

  SETUP_LEDS();
  SETUP_SWITCHES(&BUTTON_L, &BUTTON_R);
  SETUP_SPI();
  SETUP_ADC(adc);

  SETUP_DAC(&presets[0], &sgtl5000, &waveform1, &sine_fm1, &fade1);

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

#if USB_MIDI
  USB_MIDI_SETUP();
#endif

#if USB_SLIP_OSC
  USB_SLIP_OSC_SETUP();
#endif
}

//////////////////// LOOP
void loop() {

  if (loadPreset) preset_load(&presets[0], &loadPreset);
  if (savePreset) preset_save(&presets[0], &savePreset);

  update_buttons(
    &presets[0],
    &BUTTON_L,
    &BUTTON_R,
    &encoder,
    &currentMode,
    &lastMode,
    &ledIterations,
    &toggleSwitch,
    &ledTimer
  );

  update_preset(
    &presets[currentMode],
    &encoder,
    &calibrateMatrix,
    &savePreset,
    &sgtl5000,
    &ledTimer
  );

  update_leds(
    &presets[currentMode],
    &currentMode,
    &lastMode,
    &ledTimer
  );

  if (calibrateMatrix) {
    calibrateMatrix = false;
    calibrate_matrix(
      adc,
      &result,
      &offsetArray[0],
      &setDualRows[0]
    );
  }

  scan_matrix(
    adc,
    &result,
    &frameArray[0],
    &offsetArray[0],
    &setDualRows[0]
  );

#if DEBUG_ADC
  if (timerDebug >= 1000) {
    timerDebug = 0;
    print_adc(&inputFrame);
  }
#endif

  interp_matrix(
    constrain(presets[THRESHOLD].val - 10, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal),
    &interpolatedFrame,
    &inputFrame,
    &interp
  );

#if DEBUG_INTERP
  if (timerDebug >= 1000) {
    timerDebug = 0;
    print_interp(&interpolatedFrame);
  }
#endif

  find_blobs(
    presets[THRESHOLD].val, // uint8_t (Z)Threshold
    &interpolatedFrame,     // image_t (uint8_t array[NEW_FRAME] - 64*64 1D array)
    &bitmap,                // image_t (uint8_t array[NEW_FRAME] - 64*64 1D array)
    &lifo_stack,            // lifo_t
    &lifo,                  // lifo_t
    &blobs_stack,           // list_t
    &blobs,                 // list_t
    &outputBlobs            // list_t
  );

#if DEBUG_BITMAP
  if (timerDebug >= 100) {
    timerDebug = 0;
    print_bitmap(&bitmap);
  }
#endif

#if DEBUG_BLOBS
  print_blobs(&outputBlobs);
#endif

#if USB_MIDI
  if (currentMode == MIDI_LEARN) {
    blobs_usb_midi_learn(&outputBlobs, &presets[MIDI_LEARN]);
  }
  else {
    blobs_usb_midi_play(&outputBlobs);
  }
#endif

#if USB_SLIP_OSC
  blobs_usb_slipOsc(&outputBlobs, &presets[THRESHOLD]);
#endif

#if STANDALONE
  // Make some mapping
  // config.h
  //    SET_ORIGIN_X == 1
  //    SET_ORIGIN_Y == 1
  //    The sensor surface origine [X:0-Y:0] is TOP_LEFT

  for (blob_t* blob = ITERATOR_START_FROM_HEAD(&outputBlobs); blob != NULL; blob = ITERATOR_NEXT(blob)) {
    //polar_t polarCoord = polarCoordinates(blob, POLAR_X, POLAR_Y);
    //keyCode_t key = gridLayout(blob, NEW_COLS, NEW_ROWS, 20, 20, 0, 0); // ARGS [blob/gridW/gridH/stepX/stepY/posX/posY]
    //boolean togSwitchVal_A = toggle(blob, &modeSwitch_A);
    //boolean togSwitchVal_B = toggle(blob, &modeSwitch_B);
    //boolean tapSwitchVal = trigger(blob, &tapSwitch);
    //float hSliderPos = hSlider(blob, 30, 5, 50, 5); // ARGS [blob/posY/Xmin/Xmax/hSize
    //float vSliderPos = vSlider(blob, 30, 5, 50, 5); // ARGS [blob/posX/Ymin/Ymax/wSize
  }

  make_noise(
    &presets[0],
    &outputBlobs,
    &sgtl5000,
    &waveform1,
    &sine_fm1,
    &fade1
  );
#endif

#if DEBUG_FPS
  if (curentMillisFps >= 1000) {
    curentMillisFps = 0;
    Serial.println(fps);
    fps = 0;
  }
  fps++;
#endif
}
