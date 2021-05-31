/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "presets.h"
#include "scan.h"
#include "interp.h"
#include "blob.h"
#include "median.h"
#include "notes.h"
#include "mapping.h"
#include "soundCard.h"
#include "transmit.h"

#if FLASH_PLAYER
#include "player_flash.h"
#endif
#if SYNTH_PLAYER
#include "player_synth.h"
#endif
#if GRANULAR_PLAYER
#include "player_granular.h"
#endif

uint8_t interpThreshold = 5;

image_t  rawFrame;         // Input frame values
image_t  interpFrame;      // Interpolated frame values
llist_t  blobs;            // Output blobs linked list
llist_t  midiIn;           // MidiIn linked list

presetMode_t lastMode = LINE_OUT;       // Init lastMode with LINE_OUT (DEFAULT_MODE)
presetMode_t currentMode = CALIBRATE;   // Init currentMode with CALIBRATE (DEFAULT_MODE)

#if DEBUG_FPS
elapsedMillis curentMillisFps;
unsigned int fps = 0;
#endif

#if DEBUG_ADC || DEBUG_BITMAP || DEBUG_INTERP
elapsedMillis debugTimer;
#endif

//boolean loadPreset = true;
//boolean savePreset = false;

preset_t presets[7] = {
  {13, 31, 29, 0, false, false, false, LOW,  LOW },  // LINE_OUT   - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 50, 12, 0, false, false, false, HIGH, LOW },  // SIG_IN     - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 31, 17, 0, false, false, false, LOW,  HIGH},  // SIG_OUT    - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 5, 30, 10, 0, false, false, false, HIGH, HIGH},  // THRESHOLD  - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 6,  1,  0, false, false, false, NULL, NULL},  // MIDI_LEARN - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 0, 0,  0,  0, true,  true,  false, NULL, NULL},  // CALIBRATE  - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 0, 0,  0,  0, false, false, false, NULL, NULL}   // SAVE       - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
};

// MAPPING
tSwitch_t trigParam = {10, 10, 5, 1000, false};     // ARGS[posX, posY, rSize, debounceTimer, state]
tSwitch_t toggParam = {40, 30, 5, 1000, false};     // ARGS[posX, posY, rSize, debounceTimer, state]
vSlider_t vSliderParam = {10, 15, 40, 5, 0};        // ARGS[posX, Ymin, Ymax, width, val]
hSlider_t hSliderParam = {30, 15, 40, 5, 0};        // ARGS[posY, Xmin, Xmax, width, val]
cSlider_t cSlidersParam[C_SLIDERS] = {
  {   6, 4,  3.8,  5, 0},                           // ARGS[r, width, phiOffset, phiMax, val]
  {13.5, 3,  3.8, 10, 0},                           // ARGS[r, width, phiOffset, phiMax, val]
  {  20, 4,  4.8,  5, 0}                            // ARGS[r, width, phiOffset, phiMax, val]
};
ccPesets_t ccParam = {NULL, BD, 44, 1, 0};          // ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]

void setup() {
#if DEBUG_ADC || DEBUG_INTERP || DEBUG_BITMAP || DEBUG_BLOBS || DEBUG_FPS || DEBUG_ENCODER || DEBUG_BUTTONS || DEBUG_MAPPING
  Serial.begin(BAUD_RATE); // Start Serial communication using 230400 baud
  while (!Serial);
  Serial.printf("\n%s_%s", NAME, VERSION);
#endif
  LEDS_SETUP();
  SWITCHES_SETUP();
  SPI_SETUP();
  ADC_SETUP();
  SCAN_SETUP(&rawFrame);
  INTERP_SETUP(&interpFrame);
  BLOB_SETUP(&blobs);
#if USB_MIDI
  USB_MIDI_SETUP();
#endif
#if USB_SLIP_OSC
  USB_SLIP_OSC_SETUP();
#endif
#if HARDWARE_MIDI
  HARDWARE_MIDI_SETUP();
#endif
#if SYNTH_PLAYER || GRANULAR_PLAYER || FLASH_PLAYER
  SOUND_CARD_SETUP();
#endif
#if SYNTH_PLAYER
  SYNTH_PLAYER_SETUP();
#endif
#if FLASH_PLAYER
  FLASH_PLAYER_SETUP();
#endif
#if GRANULAR_PLAYER
  GRANULAR_PLAYER_SETUP();
#endif
#if HARDWARE_MIDI || SYNTH_PLAYER
  GRID_LAYOUT_SETUP();
#endif
};

//////////////////// LOOP
void loop() {
  //if (loadPreset) preset_load(&presets[0], &loadPreset); // TODO
  //if (savePreset) preset_save(&presets[0], &savePreset); // TODO
  update_buttons(&lastMode, &currentMode, &presets[0]);
  update_presets(currentMode, &presets[0]);
  update_volumes(currentMode, &presets[0]);
  update_leds(currentMode, &presets[0]);
  calibrate_matrix(&lastMode, &currentMode, &presets[0]);
  scan_matrix();
  interp_matrix(&rawFrame, &interpFrame, interpThreshold);
  find_blobs(presets[THRESHOLD].val, &interpFrame, &blobs);

  //median(&blobs);
  //getPolarCoordinates(&blobs);
  //getBlobsVelocity(&blobs);

#if USB_MIDI
  if (currentMode == MIDI_LEARN) {
    usb_midi_learn(&blobs, &presets[MIDI_LEARN]);
  }
  else {
    usb_midi_play(&blobs);
  };
#endif

#if USB_SLIP_OSC
  usb_slipOsc(&blobs);
#endif

#if HARDWARE_MIDI
  if (handleMidiInput(&midiIn)) {
    gridPopulate(&midiIn);
  };
#endif

  //gridPlay(&blobs);

  //controlChange(&blobs, &ccParam);
  //boolean toggSwitch = toggle(&blobs, &toggParam);
  //boolean trigSwitch = trigger(&blobs, &trigParam);
  //hSlider(&blobs, &hSliderParam);
  //vSlider(&blobs, &vSliderParam);
  //cSlider(&blobs, &polarCoord[0], &cSlidersParam[0]);

#if SYNTH_PLAYER
  synth_player(&blobs);
#endif
#if GRANULAR_PLAYER
  granular_player(&blobs);
#endif
#if FLASH_PLAYER
  flash_player(&blobs);
#endif

#if DEBUG_FPS
  if (curentMillisFps >= 1000) {
    curentMillisFps = 0;
    Serial.printf("\nFPS:%d\tCPU:%f\tMEM:%f", fps, AudioProcessorUsageMax(), AudioMemoryUsageMax());
    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();
    fps = 0;
  };
  fps++;
#endif

#if DEBUG_ADC
  if (debugTimer >= 1000) {
    debugTimer = 0;
    print_adc(&rawFrame);
  };
#endif
#if DEBUG_INTERP
  if (debugTimer >= 500) {
    debugTimer = 0;
    print_interp(&interpFrame);
  };
#endif
#if DEBUG_BITMAP
  if (debugTimer >= 100) {
    debugTimer = 0;
    print_bitmap();
  };
#endif
#if DEBUG_BLOBS
  print_blobs(&blobs);
#endif
};
