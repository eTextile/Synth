/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "scan.h"
#include "interp.h"
#include "midi_bus.h"
#include "usb_midi_transmit.h"
#include "allocate.h"
#include "mapping_lib.h"

#include <ArduinoJson.h>
#include <SerialFlash.h>
#include <Bounce2.h>

// The modes below can be selected using E256 built-in switches
Bounce BUTTON_L = Bounce();
Bounce BUTTON_R = Bounce();

// The levels below can be adjusted using E256 built-in encoder
Encoder e256_e(ENCODER_PIN_A, ENCODER_PIN_B);

e256_mode_t e256_m[4] = {
  { { HIGH,  LOW, false }, 500, 700, true },   // [0] RAW_MATRIX
  { { HIGH,  LOW, false }, 800, 800, true },   // [1] INTERP_MATRIX
  { { HIGH, HIGH, false }, 400, 1000, true },  // [2] BLOBS_PLAY
  { { HIGH, HIGH, false }, 1000, 1000, true }  // [3] MAPPING
};

e256_level_t e256_l[4] = {
  { { HIGH,  LOW, false },  1, 50, 12, false }, // [0]  SIG_IN     
  { {  LOW, HIGH, false },  1, 31, 17, false }, // [1]  SIG_OUT    
  { {  LOW,  LOW, false }, 13, 31, 29, false }, // [2]  LINE_OUT
  { { HIGH, HIGH, false },  2, 60,  3, false }  // [3]  THRESHOLD
};

e256_state_t e256_s[4] = {
  { {  LOW, LOW, false },  20,  20, 6  }, // [0] CALIBRATE
  { { HIGH, LOW, false },  150, 100, 4 }, // [1] DONE_ACTION
  { { HIGH, LOW, false },  25,  25, 10 }  // [2] ERROR
};

e256_control_t e256_ctr = {
  &e256_e,    // encoder_ptr
  &e256_m[0], // modes_ptr
  &e256_s[0], // state_ptr
  &e256_l[0]  // levels_ptr
};

uint8_t playMode = BLOBS_PLAY;
uint8_t lastMode = BLOBS_PLAY;
uint8_t levelMode = THRESHOLD;

uint8_t* config_ptr = NULL;
uint16_t configSize = 0;

// Her it should not compile if you didn't install the library
// [Bounce2]: https://github.com/thomasfredericks/Bounce2
// https://www.pjrc.com/teensy/interrupts.html
// https://github.com/khoih-prog/Teensy_TimerInterrupt/blob/main/examples/SwitchDebounce/SwitchDebounce.ino
inline void setup_buttons(void) {
  BUTTON_L.attach(BUTTON_PIN_L, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_R.attach(BUTTON_PIN_R, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_L.interval(25);                        // Debounce interval of 25 millis
  BUTTON_R.interval(25);                        // Debounce interval of 25 millis
};

void setup_leds(void* ptr){
  leds_t* leds = (leds_t*)ptr;
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
  digitalWrite(LED_PIN_D1, leds->D1);
  digitalWrite(LED_PIN_D2, leds->D2);
};

void set_mode(uint8_t mode) {
  e256_ctr.modes[playMode].leds.update = false;
  e256_ctr.levels[levelMode].leds.update = false;
  setup_leds(&e256_ctr.modes[mode]);
  e256_ctr.modes[mode].leds.update = true;
  playMode = mode;
  #if defined(DEBUG_MODES)
    Serial.printf("\nSET_MODE:%d", mode);
  #endif
};

void set_level(uint8_t level, uint8_t value) {
  e256_ctr.modes[playMode].leds.update = false;
  e256_ctr.encoder->write(value << 2);
  setup_leds(&e256_ctr.levels[level]);
  e256_ctr.levels[level].update = true;
  levelMode = level;
  #if defined(DEBUG_LEVELS)
    Serial.printf("\nSET_LEVEL:%d_%d", level, value);
  #endif
};

void set_state(uint8_t state) {
  setup_leds(&e256_ctr.states[state]);
  for (int i = 0; i<e256_ctr.states[state].iter; i++){
    digitalWrite(LED_PIN_D1, e256_ctr.states[state].leds.D1);
    digitalWrite(LED_PIN_D2, e256_ctr.states[state].leds.D2);
    delay(e256_ctr.states[state].timeOn);
    digitalWrite(LED_PIN_D1, !e256_ctr.states[state].leds.D1);
    digitalWrite(LED_PIN_D2, !e256_ctr.states[state].leds.D2);
    delay(e256_ctr.states[state].timeOff);
  };
  #if defined(DEBUG_STATES)
    Serial.printf("\nSET_STATE:%d", state);
  #endif
};

inline void flash_config(uint8_t* data_ptr, unsigned int size) {
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    midiInfo(ERROR_CONNECTING_FLASH);
    set_state(ERROR);
    return;
  };
  while (!SerialFlash.ready());
  SerialFlashFile flashFile;
  if (SerialFlash.exists("config.json")) {
    SerialFlash.remove("config.json"); // It doesn't reclaim the space, but it does let you create a new file with the same name
  };
  // Create a new file and open it for writing
  if (SerialFlash.create("config.json", size)) {
    flashFile = SerialFlash.open("config.json");
    if (!flashFile) {
      midiInfo(ERROR_WHILE_OPEN_FLASH_FILE);
      set_state(ERROR);
      return;
    };
  }
  else {
    midiInfo(ERROR_FLASH_FULL);
    set_state(ERROR);
    return;
  };
  if (size < FLASH_SIZE) {
    flashFile.write(data_ptr, size);
    flashFile.close();
    midiInfo(DONE_FLASH_CONFIG_WRITE);
    set_state(DONE_ACTION);
  } else {
    midiInfo(ERROR_FILE_TO_BIG);
    set_state(ERROR);
  };
};

// Selec the current mode and perform the matrix sensor calibration 
inline void update_buttons(void) {
  BUTTON_L.update();
  BUTTON_R.update();
  // ACTION: BUTTON_L short press
  // FONCTION: CALIBRATE THE SENSOR MATRIX
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() < LONG_HOLD) {
    matrix_calibrate();
  };
  // ACTION: BUTTON_L long press
  // FONCTION: FLASH THE CONFIG FILE (config.json)
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() > LONG_HOLD) {
    flash_config(sysEx_data_ptr, sysEx_dataSize);
  };
  // ACTION: BUTTON_R long press
  // FONCTION_A: BLOBS_PLAY (send all blob values over MIDI format)
  // FONCTION_B: BLOBS_LEARN (send blob values separately for Max4Live MIDI_LEARN)
  // LEDs: blink alternately, slow for playing mode and fast or learning mode
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() > LONG_HOLD) {
    set_mode(MAPPING_LIB);
  };
  // ACTION: BUTTON_R short press
  // FONCTION: SELECT_LEVEL
  // levels[0] = SIG_IN
  // levels[1] = SIG_OUT
  // levels[2] = LINE_OUT
  // levels[3] = THRESHOLD
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() < LONG_HOLD) {
    levelMode = (levelMode + 1) % 4; // Loop into level modes
    set_level(levelMode, e256_ctr.levels[levelMode].val);
  };
};

// Levels values adjustment using rotary encoder
// TODO: add interrupt
inline boolean read_encoder(uint8_t level) {
  uint8_t val = e256_ctr.encoder->read() >> 2;
  if (val != e256_ctr.levels[level].val) {
    if (val > e256_ctr.levels[level].maxVal) {
      e256_ctr.encoder->write(e256_ctr.levels[level].maxVal << 2);
      return false;
    }
    else if (val < e256_ctr.levels[level].minVal) {
      e256_ctr.encoder->write(e256_ctr.levels[level].minVal << 2);
      return false;
    }
    else {
      e256_ctr.levels[level].val = val;
      e256_ctr.levels[level].leds.update = true;
      return true;
    };
  }
  else {
    return false;
  };
};

// Update levels[level] of each mode using the rotary encoder
inline void update_encoder() {
  if (read_encoder(levelMode)) {
    set_level(levelMode, e256_ctr.levels[levelMode].val);
  };
};

inline void blink_leds(uint8_t mode) {
  static uint32_t ledsTimeStamp = 0;
  if (e256_ctr.modes[mode].leds.update) {
    if (millis() - ledsTimeStamp < e256_ctr.modes[mode].timeOn && e256_ctr.modes[mode].toggle == true ) {
      e256_ctr.modes[mode].toggle = false;
      digitalWrite(LED_PIN_D1, e256_ctr.modes[mode].leds.D1);
      digitalWrite(LED_PIN_D2, e256_ctr.modes[mode].leds.D2);
    }
    else if (millis() - ledsTimeStamp > e256_ctr.modes[mode].timeOn && e256_ctr.modes[mode].toggle == false) {
      e256_ctr.modes[mode].toggle = true;
      digitalWrite(LED_PIN_D1, !e256_ctr.modes[mode].leds.D1);
      digitalWrite(LED_PIN_D2, !e256_ctr.modes[mode].leds.D2);
    }
    else if (millis() - ledsTimeStamp > e256_ctr.modes[mode].timeOn + e256_ctr.modes[mode].timeOff) {
    ledsTimeStamp = millis();
    };
  };
};

inline void fade_leds(uint8_t level) {
  if (e256_ctr.levels[level].leds.update) {
    e256_ctr.levels[level].leds.update = false;
    uint8_t ledVal = constrain(map(e256_ctr.levels[level].val, e256_ctr.levels[level].minVal, e256_ctr.levels[level].maxVal, 0, 255), 0, 255);
    analogWrite(LED_PIN_D1, abs(255 - ledVal));
    analogWrite(LED_PIN_D2, ledVal);
  };
};

// Update LEDs according to the mode and rotary encoder values
inline void update_leds(void) {
  fade_leds(levelMode);
  blink_leds(playMode);
};

//////////////////////////////////////// LOAD CONFIG
inline bool config_load_mapping_triggers(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  mapping_triggers_alloc(config.size());
  for (uint8_t i = 0; i < mapp_trigs; i++) {
    mapp_trigsParams[i].rect.from.x = config[i]["Xmin"];
    mapp_trigsParams[i].rect.to.x = config[i]["Xmax"];
    mapp_trigsParams[i].rect.from.y = config[i]["Ymin"];
    mapp_trigsParams[i].rect.to.y = config[i]["Ymax"];
    mapp_trigsParams[i].note = config[i]["note"];
  };
  return true;
};

inline bool config_load_mapping_toggles(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  }
  mapping_toggles_alloc(config.size());
  for (uint8_t i = 0; i < mapp_togs; i++) {
    mapp_togsParams[i].rect.from.x = config[i]["Xmin"];
    mapp_togsParams[i].rect.to.x = config[i]["Xmax"];
    mapp_togsParams[i].rect.from.y = config[i]["Ymin"];
    mapp_togsParams[i].rect.to.y = config[i]["Ymax"];
    mapp_togsParams[i].note = config[i]["note"];
  }
  return true;
}

inline bool config_load_mapping_vSliders(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  mapping_vSliders_alloc(config.size());
  for (uint8_t i = 0; i < mapp_vSliders; i++) {
    mapp_vSlidersParams[i].rect.from.x = config[i]["Xmin"];
    mapp_vSlidersParams[i].rect.to.x = config[i]["Xmax"];
    mapp_vSlidersParams[i].rect.from.y = config[i]["Ymin"];
    mapp_vSlidersParams[i].rect.to.y = config[i]["Ymax"];
    mapp_vSlidersParams[i].CC = config[i]["CC"];
  };
  return true;
};

inline bool config_load_mapping_hSliders(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  mapping_hSliders_alloc(config.size());
  for (uint8_t i = 0; i < mapp_hSliders; i++) {
    mapp_hSlidersParams[i].rect.from.x = config[i]["Xmin"];
    mapp_hSlidersParams[i].rect.to.x = config[i]["Xmax"];
    mapp_hSlidersParams[i].rect.from.y = config[i]["Ymin"];
    mapp_hSlidersParams[i].rect.to.y = config[i]["Ymax"];
    mapp_hSlidersParams[i].CC = config[i]["CC"];
  };
  return true;
};

inline bool config_load_mapping_circles(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  }
  mapping_circles_alloc(config.size());
  for (uint8_t i = 0; i < mapp_circles; i++) {
    mapp_circlesParams[i].center.x = config[i]["CX"];
    mapp_circlesParams[i].center.y = config[i]["CY"];
    mapp_circlesParams[i].radius = config[i]["radius"];
    mapp_circlesParams[i].offset = config[i]["offset"];
    mapp_circlesParams[i].CCr = config[i]["CCradius"];
    mapp_circlesParams[i].CCt = config[i]["CCtheta"];
  };
  return true;
}

inline bool config_load_mapping_touchpads(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  mapping_touchpads_alloc(config.size());
  for (uint8_t i = 0; i < mapp_touchpads; i++) {
    mapp_touchpadsParams[i].rect.from.x = config[i]["Xmin"];
    mapp_touchpadsParams[i].rect.to.x = config[i]["Xmax"];
    mapp_touchpadsParams[i].rect.from.y = config[i]["Ymin"];
    mapp_touchpadsParams[i].rect.to.y = config[i]["Ymax"];
    mapp_touchpadsParams[i].CCx = config[i]["CCx"];     // Blob X axis MIDI cChange mapping
    mapp_touchpadsParams[i].CCy = config[i]["CCy"];     // Blob Y axis MIDI cChange mapping
    mapp_touchpadsParams[i].CCz = config[i]["CCz"];     // Blob Z axis MIDI cChange mapping
    mapp_touchpadsParams[i].CCs = config[i]["CCs"];     // Blob state MIDI cChange mapping
    mapp_touchpadsParams[i].CCvxy = config[i]["CCvxy"]; // Blob XY velocity MIDI cChange mapping
    mapp_touchpadsParams[i].CCvz = config[i]["CCvz"];   // Blob Z velocity MIDI cChange mapping
 };
  return true;
};

inline bool config_load_mapping_polygons(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  mapping_polygons_alloc(config.size());
  for (uint8_t i = 0; i < mapp_polygons; i++) {
    mapp_polygonsParams[i].point_cnt = config[i]["cnt"];
    for (uint8_t j = 0; j < config[i]["cnt"]; j++) {
      mapp_polygonsParams[i].point[j].x = (float)config[i]["point"][j]["X"];
      mapp_polygonsParams[i].point[j].y = (float)config[i]["point"][j]["Y"];
    };
  };
  return true;
};

bool config_load_mapping(const JsonObject &config) {
  if (config.isNull()) {
    midiInfo(ERROR_LOADING_GONFIG_FAILED);
    set_state(ERROR);
    return false;
  };
  if (!config_load_mapping_triggers(config["triggers"])) {
    midiInfo(ERROR_LOADING_GONFIG_FAILED);
    set_state(ERROR);
    return false;
  };
  if (!config_load_mapping_toggles(config["toggles"])) {
    midiInfo(ERROR_LOADING_GONFIG_FAILED);
    set_state(ERROR);
    return false;
  };
  if (!config_load_mapping_vSliders(config["vSliders"])) {
    midiInfo(ERROR_LOADING_GONFIG_FAILED);
    set_state(ERROR);
    return false;
  };
  if (!config_load_mapping_hSliders(config["hSliders"])) {
    midiInfo(ERROR_LOADING_GONFIG_FAILED);
    set_state(ERROR);
    return false;
  };
  if (!config_load_mapping_circles(config["circles"])) {
    midiInfo(ERROR_LOADING_GONFIG_FAILED);
    set_state(ERROR);
    return false;
  };
  if (!config_load_mapping_touchpads(config["touchpad"])) {
    midiInfo(ERROR_LOADING_GONFIG_FAILED);
    set_state(ERROR);
    return false;
  };
  if (!config_load_mapping_polygons(config["polygons"])) {
    midiInfo(ERROR_LOADING_GONFIG_FAILED);
    set_state(ERROR);
    return false;
  };
  return true;
};

void load_config(uint8_t* data_ptr, uint8_t msg) {
  StaticJsonDocument<2048> config;
  DeserializationError err = deserializeJson(config, data_ptr);
  if (err) {
    midiInfo(ERROR_WAITING_FOR_GONFIG);
    set_state(ERROR);
    return;
  };
  if (!config_load_mapping(config["mapping"])) {
    return;
  } else {
    midiInfo(msg);
    set_state(DONE_ACTION);
  };
};

inline void load_flash_config() {
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    midiInfo(ERROR_CONNECTING_FLASH);
    set_state(ERROR);
    return;
  };
  SerialFlashFile configFile = SerialFlash.open("config.json");
  if (configFile) {
    configSize = configFile.size();
    config_ptr = allocate(config_ptr, configSize);
    configFile.read(config_ptr, configSize);
    load_config(config_ptr, DONE_FLASH_CONFIG_LOAD);
  }
  else {
    midiInfo(ERROR_NO_CONFIG_FILE);
    set_state(ERROR);
  };
};

void CONFIG_SETUP(void){
  setup_buttons();
  load_flash_config();
  matrix_calibrate();
};

void update_controls(void){
  update_buttons();
  update_encoder();
  update_leds();
};