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
#include "mapping.h"
#include "transmit.h"

#include "soundCard.h"
#include "player_synth.h"
#include "player_flash.h"
#include "player_granular.h"

uint8_t rawFrameArray[RAW_FRAME] = {0};               // 1D Array to store E256 ofseted analog input values
uint8_t interpFrameArray[NEW_FRAME] = {0};            // 1D Array to store E256 bilinear interpolated values

median_t blobMedian[MAX_SYNTH] = {{0}, {0}, 0};       // 1D ...
velocity_t blobVelocity[MAX_SYNTH] = {0, 0, 0, 0, 0}; // 1D Array to store XY & Z blobs velocity

interp_t interp;           // Interpolation parameters structure
image_t  rawFrame;         // Input frame values
image_t  interpFrame;      // Interpolated frame values
llist_t  blobs;            // Output blobs linked list
llist_t  midiIn_stack;     // MidiIn free nodes linked list
llist_t  midiInllist;      // MidiIn linked list

AudioInputI2S                     i2s_IN;
AudioOutputI2S                    i2s_OUT;
AudioControlSGTL5000              sgtl5000;

#if FLASH_PLAYER
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
#if FLASH_PLAYER
AudioConnection                   patchCord27(playFlashRaw, 0, mix_3, 2);
#endif
AudioConnection                   patchCord28(mix_3, 0, i2s_OUT, 0);
AudioConnection                   patchCord29(mix_3, 0, i2s_OUT, 1);

synth_t allSynth[MAX_SYNTH] = {
  {&wf_1, &fm_1, &fade_1, &mix_1},
  {&wf_2, &fm_2, &fade_2, &mix_1},
  {&wf_3, &fm_3, &fade_3, &mix_1},
  {&wf_4, &fm_4, &fade_4, &mix_1},
  {&wf_5, &fm_5, &fade_5, &mix_2},
  {&wf_6, &fm_6, &fade_6, &mix_2},
  {&wf_7, &fm_7, &fade_7, &mix_2},
  {&wf_8, &fm_8, &fade_8, &mix_2}
};
#endif

#if GRANULAR_PLAYER
AudioEffectGranular               granular;
AudioConnection                   patchCord1(i2s_IN, 0, granular, 0);
AudioConnection                   patchCord2(granular, 0, i2s_OUT, 0);
AudioConnection                   patchCord3(granular, 0, i2s_OUT, 1);
#endif

presetMode_t lastMode = LINE_OUT;       // Init lastMode with LINE_OUT (DEFAULT_MODE)
presetMode_t currentMode = CALIBRATE;   // Init currentMode with CALIBRATE (DEFAULT_MODE)

#if DEBUG_FPS
elapsedMillis curentMillisFps;
unsigned int fps = 0;
#endif

#if DEBUG_ADC || DEBUG_BITMAP || DEBUG_INTERP
elapsedMillis debugTimer;
#endif

boolean loadPreset = true;
boolean savePreset = false;

preset_t presets[7] = {
  {13, 31, 29, 0, false, false, false, LOW,  LOW }, // LINE_OUT   - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 50, 12, 0, false, false, false, HIGH, LOW }, // SIG_IN     - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 31, 17, 0, false, false, false, LOW,  HIGH}, // SIG_OUT    - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 60, 15, 0, false, false, false, HIGH, HIGH}, // THRESHOLD  - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 6,  1,  0, false, false, false, NULL, NULL}, // MIDI_LEARN - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 0, 0,  0,  0, true,  true,  false, NULL, NULL}, // CALIBRATE  - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 0, 0,  0,  0, false, false, false, NULL, NULL}  // SAVE       - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
};

// MAPPING
tSwitch_t tapSwitch = {10, 10, 5, 1000, false};           // ARGS[posX, posY, rSize, debounceTimer, state]
tSwitch_t modeSwitch = {40, 30, 5, 1000, false};          // ARGS[posX, posY, rSize, debounceTimer, state]

int8_t lastKey[MAX_SYNTH] = {0};                          // 1D Array to store last keys pressed
squareKey_t keyArray[GRID_KEYS] = {0, 0, 0, 0};           // 1D Array of struct squareKey_t to store pre-compute key positions ARGS[Xmin, Xmax, Ymin, Ymax]
midiNode_t midiInArray[MAX_SYNTH] = {0, 0, 0};            // 1D Array to store incoming midi notes

grid_t grid = {&lastKey[0], &keyArray[0], &midiInllist};  // ARGS[blobKeyPress, KeyPress, midiNotes]

polar_t polarCoord[MAX_SYNTH];                            // 1D Array of struct polar_t to store blobs polar coordinates

vSlider_t vSlider_A = {10, 15, 40, 5, 0};                 // ARGS[posX, Ymin, Ymax, width, val]
hSlider_t hSlider_A = {30, 15, 40, 5, 0};                 // ARGS[posY, Xmin, Xmax, width, val]

cSlider_t cSliders[C_SLIDERS] = {
  {   6, 4,  3.8,  5, 0},                                 // ARGS[r, width, phiOffset, phiMax, val]
  {13.5, 3,  3.8, 10, 0},                                 // ARGS[r, width, phiOffset, phiMax, val]
  {  20, 4,  4.8,  5, 0}                                  // ARGS[r, width, phiOffset, phiMax, val]
};

ccPesets_t ccPeset = {NULL, BD, 44, 1, 0};                // ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]

void setup() {
#if DEBUG_ADC || DEBUG_INTERP || DEBUG_BLOBS || DEBUG_SFF_BITMAP || DEBUG_FPS || DEBUG_ENCODER || DEBUG_BUTTONS
  Serial.begin(BAUD_RATE); // Start Serial communication using 230400 baud
  while (!Serial);
  Serial.printf("\n%s_%s", NAME, VERSION);
  delay(1000);
#endif
  LEDS_SETUP();
  SWITCHES_SETUP();
  SPI_SETUP();
  ADC_SETUP();
  INTERP_SETUP(
    &rawFrameArray[0],    // uint8_t*
    &rawFrame,            // image_t*
    &interpFrameArray[0], // uint8_t*
    &interpFrame,         // image_t*
    &interp               // interp_t*
  );
  BLOB_SETUP(&blobs);
#if USB_MIDI
  USB_MIDI_SETUP();
#endif
#if USB_SLIP_OSC
  USB_SLIP_OSC_SETUP();
#endif
#if HARDWARE_MIDI
  HARDWARE_MIDI_SETUP();
  llist_raz(&midiIn_stack);
  midiIn_llist_init(&midiIn_stack, &midiInArray[0], MAX_SYNTH);
  llist_raz(&midiInllist);
#endif
#if SYNTH_PLAYER || GRANULAR_PLAYER || FLASH_PLAYER
  SOUND_CARD_SETUP(&sgtl5000);
#endif
#if SYNTH_PLAYER
  SYNTH_PLAYER_SETUP(&allSynth[0]);
#endif
#if FLASH_PLAYER
  FLASH_PLAYER_SETUP();
#endif
#if GRANULAR_PLAYER
  GRANULAR_PLAYER_SETUP(&granular);
#endif
  GRID_LAYOUT_SETUP(&keyArray[0]);
}

//////////////////// LOOP
void loop() {

  //if (loadPreset) preset_load(&presets[0], &loadPreset); // TODO
  //if (savePreset) preset_save(&presets[0], &savePreset); // TODO

  update_buttons(
    &lastMode,
    &currentMode,
    &presets[0]
  );

  update_presets(
    currentMode,
    &presets[0],
    &interp
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
    &lastMode,
    &currentMode,
    &presets[0]
  );

  scan_matrix(&rawFrameArray[0]);

#if DEBUG_ADC
  if (debugTimer >= 1000) {
    debugTimer = 0;
    print_adc(&rawFrame);
  }
#endif

  interp_matrix(
    &interp,
    &rawFrame,
    &interpFrame
  );

#if DEBUG_INTERP
  if (debugTimer >= 1000) {
    debugTimer = 0;
    print_interp(&interpFrame);
  }
#endif

  find_blobs(
    presets[THRESHOLD].val, // uint8_t (Z)Threshold
    &interpFrame,           // image_ptr (uint8_t array[NEW_FRAME] - 64*64 1D array)
    &blobs                  // llist_ptr
  );

  median(&blobs, &blobMedian[0]);              // ARGS[llist_ptr, median_ptr]
  getPolarCoordinates(&blobs, &polarCoord[0]); // ARGS[llist_ptr, polar_ptr]
  getBlobsVelocity(&blobs, &blobVelocity[0]);  // ARGS[llist_ptr, velocity_ptr]

#if DEBUG_BITMAP
  if (debugTimer >= 100) {
    debugTimer = 0;
    print_bitmap();
  }
#endif

#if DEBUG_BLOBS
  print_blobs(&blobs);
#endif

#if USB_MIDI
  if (currentMode == MIDI_LEARN) {
    usb_midi_learn(&blobs, &presets[MIDI_LEARN]);
  }
  else {
    usb_midi_play(&blobs);
  }
#endif

#if USB_SLIP_OSC
  usb_slipOsc(&blobs);
#endif

  // Make some mapping
  // The sensor surface origine [0:0] is TOP_LEFT

#if HARDWARE_MIDI
  gridLayout(&blobs, &grid);                              // ARGS[llist_ptr, gridLayout_ptr]
  //gridGapLayout(&blobs, &grid);                         // ARGS[llist_ptr, gridLayout_ptr]
  //controlChangeMapping(&blobs, &ccPesets);              // ARGS[llist_ptr, ccccPesets_ptr]
#endif

  //boolean togSwitchVal = toggle(&blobs, &modeSwitch);   // ARGS[llist_ptr, switch_ptr]
  //boolean tapSwitchVal = trigger(&blobs, &tapSwitch);   // ARGS[llist_ptr, switch_ptr]

  //hSlider(&blobs, &hSlider_A);                          // ARGS[llist_ptr, hSlider_ptr]
  //vSlider(&blobs, &vSlider_A);                          // ARGS[llist_ptr, vSlider_ptr]
  //cSlider(&blobs, &polarCoord[0], &cSliders[0]);        // ARGS[llist_ptr, polar_ptr, cSliders_ptr]

#if SYNTH_PLAYER
  synth_player(&allSynth[0], &blobs);
#endif

#if GRANULAR_PLAYER
  granular_player(&granular, &blobs);
#endif

#if FLASH_PLAYER
  flash_player(&playFlashRaw, &blobs);
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
