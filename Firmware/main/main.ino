/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/
#include <Audio.h>              // https://github.com/PaulStoffregen/Audio
#include <Wire.h>               // https://github.com/PaulStoffregen/Wire
#include <SPI.h>                // https://github.com/PaulStoffregen/SPI
#include <SD.h>                 // https://github.com/PaulStoffregen/SD
#include <SerialFlash.h>        // https://github.com/PaulStoffregen/SerialFlash

#include "config.h"
#include "presets.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"
#include "median.h"
#include "transmit.h"
#include "mapping.h"

#include "soundCard.h"
#include "player_granular.h"
#include "player_synth.h"
#include "player_raw.h"

// Array to store all parameters used to configure the two 8:1 analog multiplexeurs
// Each byte |ENA|A|B|C|ENA|A|B|C|
uint8_t setDualRows[DUAL_COLS] = {
#if SET_ORIGIN_X
  0x33, 0x00, 0x11, 0x22, 0x44, 0x66, 0x77, 0x55
#else
  0x55, 0x77, 0x66, 0x44, 0x22, 0x11, 0x00, 0x33
#endif
};

uint8_t offsetArray[RAW_FRAME] = {0};        // 1D Array to store E256 smallest values
uint8_t frameArray[RAW_FRAME] = {0};         // 1D Array to store E256 ofseted analog input values
uint8_t bitmapArray[NEW_FRAME] = {0};        // 1D Array to store E256 binary values (16*16) array containing (64*64) values
uint8_t interpFrameArray[NEW_FRAME] = {0};   // 1D Array to store E256 bilinear interpolated values
xylr_t lifoArray[LIFO_MAX_NODES] = {0};      // 1D Array to store E256 lifo nodes
blob_t blobArray[MAX_NODES] = {0};           // 1D Array to store E256 blobs

image_t  inputFrame;            // Input frame values structure
interp_t interp;                // Interpolation parameters structure
image_t  interpolatedFrame;     // Interpolated frame structure
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

AudioInputI2S                     i2s_IN;
AudioOutputI2S                    i2s_OUT;
AudioControlSGTL5000              sgtl5000;

#if RAW_PLAYER
AudioPlaySerialflashRaw           playFlashRaw;
#endif

#if SYNTH_PLAYER
AudioSynthWaveform                wf_1;
AudioSynthWaveform                wf_2;
AudioSynthWaveform                wf_3;
AudioSynthWaveform                wf_4;
AudioSynthWaveform                wf_5;
AudioSynthWaveform                wf_6;
AudioSynthWaveform                wf_7;
AudioSynthWaveform                wf_8;
AudioSynthWaveformSineModulated   fm_1;
AudioSynthWaveformSineModulated   fm_2;
AudioSynthWaveformSineModulated   fm_3;
AudioSynthWaveformSineModulated   fm_4;
AudioSynthWaveformSineModulated   fm_5;
AudioSynthWaveformSineModulated   fm_6;
AudioSynthWaveformSineModulated   fm_7;
AudioSynthWaveformSineModulated   fm_8;
AudioEffectFade                   fade_1;
AudioEffectFade                   fade_2;
AudioEffectFade                   fade_3;
AudioEffectFade                   fade_4;
AudioEffectFade                   fade_5;
AudioEffectFade                   fade_6;
AudioEffectFade                   fade_7;
AudioEffectFade                   fade_8;
AudioMixer4                       mix_1;
AudioMixer4                       mix_2;
AudioMixer4                       mix_3;

AudioConnection                   patchCord1(wf_1, fm_1);
AudioConnection                   patchCord2(wf_2, fm_2);
AudioConnection                   patchCord3(wf_3, fm_3);
AudioConnection                   patchCord4(wf_4, fm_4);
AudioConnection                   patchCord5(wf_5, fm_5);
AudioConnection                   patchCord6(wf_6, fm_6);
AudioConnection                   patchCord7(wf_7, fm_7);
AudioConnection                   patchCord8(wf_8, fm_8);
AudioConnection                   patchCord9(fm_1, fade_1);
AudioConnection                   patchCord10(fm_2, fade_2);
AudioConnection                   patchCord11(fm_3, fade_3);
AudioConnection                   patchCord12(fm_4, fade_4);
AudioConnection                   patchCord13(fm_5, fade_5);
AudioConnection                   patchCord14(fm_6, fade_6);
AudioConnection                   patchCord15(fm_7, fade_7);
AudioConnection                   patchCord16(fm_8, fade_8);
AudioConnection                   patchCord17(fade_1, 0, mix_1, 0);
AudioConnection                   patchCord18(fade_2, 0, mix_1, 1);
AudioConnection                   patchCord19(fade_3, 0, mix_1, 2);
AudioConnection                   patchCord20(fade_4, 0, mix_1, 3);
AudioConnection                   patchCord21(fade_5, 0, mix_2, 0);
AudioConnection                   patchCord22(fade_6, 0, mix_2, 1);
AudioConnection                   patchCord23(fade_7, 0, mix_2, 2);
AudioConnection                   patchCord24(fade_8, 0, mix_2, 3);

AudioConnection                   patchCord25(mix_1, 0, mix_3, 0);
AudioConnection                   patchCord26(mix_2, 0, mix_3, 1);
#if RAW_PLAYER
AudioConnection                   patchCord27(playFlashRaw, 0, mix_3, 2);
#endif
AudioConnection                   patchCord28(mix_3, 0, i2s_OUT, 0);
AudioConnection                   patchCord29(mix_3, 0, i2s_OUT, 1);

synth_t allSynth[8] = {
  {&wf_1, &fm_1, &fade_1, &mix_1, 0},
  {&wf_2, &fm_2, &fade_2, &mix_1, 0},
  {&wf_3, &fm_3, &fade_3, &mix_1, 0},
  {&wf_4, &fm_4, &fade_4, &mix_1, 0},
  {&wf_5, &fm_5, &fade_5, &mix_2, 0},
  {&wf_6, &fm_6, &fade_6, &mix_2, 0},
  {&wf_7, &fm_7, &fade_7, &mix_2, 0},
  {&wf_8, &fm_8, &fade_8, &mix_2, 0}
};
#endif

#if GRANULAR_PLAYER
AudioEffectGranular               granular;
AudioConnection                   patchCord1(i2s_IN, 0, granular, 0);
AudioConnection                   patchCord2(granular, 0, i2s_OUT, 0);
AudioConnection                   patchCord3(granular, 0, i2s_OUT, 1);
#endif

presetMode_t currentMode = CALIBRATE;   // Init currentMode with CALIBRATE (DEFAULT_MODE)
presetMode_t lastMode = LINE_OUT;      // Init lastMode with LINE_OUT (DEFAULT_MODE)

#if DEBUG_FPS
elapsedMillis curentMillisFps;
unsigned int fps = 0;
#endif

#if DEBUG_ADC || DEBUG_BITMAP || DEBUG_INTERP
elapsedMillis timerDebug;
#endif

boolean loadPreset = true;
boolean savePreset = false;

preset_t presets[7] = {
  {13, 31, 29, 0, false, false, false, LOW,  LOW }, // LINE_OUT   - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 50, 12, 0, false, false, false, HIGH, LOW }, // SIG_IN     - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 31, 17, 0, false, false, false, LOW,  HIGH}, // SIG_OUT    - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 60, 10, 0, false, false, false, HIGH, HIGH}, // THRESHOLD  - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 6,  1,  0, false, false, false, NULL, NULL}, // MIDI_LEARN - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 0, 0,  0,  0, true,  true,  true,  NULL, NULL}, // CALIBRATE  - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 0, 0,  0,  0, false, false, false, NULL, NULL}  // SAVE       - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
};

uint8_t interpThreshold = 10;

median_t medianStorage[MAX_BLOBS] {
  {true, {0}, {0}, 0},
  {true, {0}, {0}, 0},
  {true, {0}, {0}, 0},
  {true, {0}, {0}, 0},
  {true, {0}, {0}, 0},
  {true, {0}, {0}, 0},
  {true, {0}, {0}, 0},
  {true, {0}, {0}, 0}
};

// MAPPING
//tSwitch_t tapSwitch = {10, 10, 5, 1000, false};         // ARGS[posX, posY, rSize, debounceTimer, state]
//tSwitch_t modeSwitch = {40, 30, 5, 1000, false};        // ARGS[posX, posY, rSize, debounceTimer, state]

squareKey_t keyArray[GRID_KEYS] = {0, 0, 0, 0};           // 1D Array of struct squareKey_t to store pre-compute key positions
int8_t keyPressed[MAX_BLOBS] = {0};                       // 1D Array to store pressed keys
int8_t midiIN[20] = {127, 63, 44};                        // 1D Array to store incoming midi notes

grid_t grid = {&keyArray[0], &keyPressed[0], &midiIN[0]}; // ARGS[blobKeyPress, lastBlobKeyPress, debounceTime, midiNotes]

polar_t polarCoord[MAX_BLOBS];                            // 1D Array of struct polar_t to store blobs polar coordinates

vSlider_t vSlider_A = {10, 15, 40, 5, 0};                 // ARGS[posX, Ymin, Ymax, width, val]
hSlider_t hSlider_A = {10, 15, 40, 5, 0};                 // ARGS[posY, Xmin, Xmax, width, val]

cSlider_t cSliders[C_SLIDERS] = {
  {   6, 4,  3.8,  5, 0},                                 // ARGS[r, width, phiOffset, phiMax, val]
  {13.5, 3,  3.8, 10, 0},                                 // ARGS[r, width, phiOffset, phiMax, val]
  {  20, 4,  4.8,  5, 0}                                  // ARGS[r, width, phiOffset, phiMax, val]
};

velocity_t blobVelocity[MAX_BLOBS];                       // 1D Array of struct velocity_t to store blobs velocity

ccPesets_t ccPesets = {NULL, BD, 44, 1, 0};               // ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]

int16_t granularMemory[GRANULAR_MEMORY_SIZE] = {0};      //

void setup() {

#if DEBUG_ADC || DEBUG_INTERP || DEBUG_BLOBS || DEBUG_SFF_BITMAP || DEBUG_FPS
  Serial.begin(BAUD_RATE); // Start Serial communication using 230400 baud
#endif

  SETUP_LEDS();

  SETUP_SWITCHES(&BUTTON_L, &BUTTON_R);
  SETUP_SPI();
  SETUP_ADC(adc);
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
#if MIDI_USB
  SETUP_MIDI_USB();
#endif
#if SLIP_OSC
  SETUP_SLIP_OSC();
#endif
#if MIDI_HARDWARE
  SETUP_MIDI_HARDWARE();
#endif
#if SYNTH_PLAYER || GRANULAR_PLAYER || RAW_PLAYER
  SETUP_SOUND_CARD(&sgtl5000);
#endif
#if SYNTH_PLAYER
  SETUP_SYNTH(&allSynth[0]);
#endif
#if RAW_PLAYER
  SETUP_FLASH_PLAYER();
#endif
#if GRANULAR_PLAYER
  SETUP_GRANULAR(&granular, &granularMemory[0]);
#endif

  SETUP_GRID_LAYOUT(&keyArray[0]);
}

//////////////////// LOOP
void loop() {

  //if (loadPreset) preset_load(&presets[0], &loadPreset); // TODO
  //if (savePreset) preset_save(&presets[0], &savePreset); // TODO

  update_buttons(
    &BUTTON_L,
    &BUTTON_R,
    &encoder,
    &presets[0],
    &currentMode,
    &lastMode
  );

  update_presets(
    currentMode,
    &presets[0],
    &encoder,
    &interpThreshold
  );

  update_volumes(
    currentMode,
    &presets[0],
    &sgtl5000
  );

  update_leds(
    currentMode,
    &presets[0]
  );

  calibrate_matrix(
    &currentMode,
    &lastMode,
    &presets[0],
    adc,
    &result,
    &offsetArray[0],
    &setDualRows[0]
  );

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
    interpThreshold,
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

  //median(&outputBlobs, &medianStorage[0]);

#if DEBUG_BITMAP
  if (timerDebug >= 100) {
    timerDebug = 0;
    print_bitmap(&bitmap);
  }
#endif

#if DEBUG_BLOBS
  print_blobs(&outputBlobs);
#endif

#if MIDI_USB
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

  // Make some mapping
  // The sensor surface origine is TOP_LEFT
  // config.h
  //    SET_ORIGIN_X == 1
  //    SET_ORIGIN_Y == 1

#if MIDI_HARDWARE
  //gridLayout(&outputBlobs, &grid);                            // ARGS[llist_ptr, gridLayout_ptr]
  gridGapLayout(&outputBlobs, &grid);                           // ARGS[llist_ptr, gridLayout_ptr]
  //controlChangeMapping(&outputBlobs, &ccPesets);              // ARGS[llist_ptr, ccPesets_ptr]
#endif

  //getVelocity(&outputBlobs, &blobVelocity[0]);                // ARGS[llist_ptr, blobVelocity_ptr]
  //hSlider(&outputBlobs, &hSlider_A);                          // ARGS[llist_ptr, hSlider_ptr]
  //vSlider(&outputBlobs, &vSlider_A);                          // ARGS[llist_ptr, vSlider_ptr]
  //getPolarCoordinates(&outputBlobs, &polarCoord[0]);          // ARGS[llist_ptr, polar_ptr]
  //cSlider(&outputBlobs, &polarCoord[0], &cSliders[0]);        // ARGS[llist_ptr, polar_ptr, cSliders_ptr]
  //boolean togSwitchVal = toggle(&outputBlobs, &modeSwitch);   // ARGS[llist_ptr, switch_ptr]
  //boolean tapSwitchVal = trigger(&outputBlobs, &tapSwitch);   // ARGS[llist_ptr, switch_ptr]

#if SYNTH_PLAYER
  synth_player(&outputBlobs, &allSynth[0]);
#endif

#if GRANULAR_PLAYER
  granular_player(&outputBlobs, &granular, &granularMemory[0]);
#endif

#if RAW_PLAYER
  play_raw(&outputBlobs, &playFlashRaw);
#endif

#if DEBUG_FPS
  if (curentMillisFps >= 1000) {
    curentMillisFps = 0;
    Serial.printf("\nFPS:%d\tCPU:%f\tMEM:%f", fps, AudioProcessorUsageMax(), AudioMemoryUsageMax());
    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();
    fps = 0;
  }
  fps++;
#endif
}
