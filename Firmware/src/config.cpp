/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "interp.h"
#include "midi_bus.h"
#include "usb_midi_transmit.h"
#include "allocate.h"
#include "mapping_lib.h"

#include <ArduinoJson.h>
#include <SerialFlash.h>
#include <Bounce2.h>

#define BUTTON_PIN_L      2
#define BUTTON_PIN_R      3
#define ENCODER_PIN_A     22
#define ENCODER_PIN_B     9
#define LONG_HOLD         1500
#define CALIBRATE_ITER    10

// The modes below can be selected using E256 built-in switches
Bounce BUTTON_L = Bounce();
Bounce BUTTON_R = Bounce();

// The levels below can be adjusted using E256 built-in encoder
Encoder e256_e(ENCODER_PIN_A, ENCODER_PIN_B);

e256_mode_t e256_m[9] = {
  { { HIGH,  LOW, false }, 200, 500, true, false },    // [0] LOAD_CONFIG
  { { HIGH, HIGH, false }, 150, 900, true, false },    // [1] FLASH_CONFIG
  { {  LOW,  LOW, false },  30,  30, true, false },    // [2] CALIBRATE
  { { HIGH,  LOW, false }, 500, 500, true, false },    // [3] BLOBS_PLAY
  { { HIGH,  LOW, false }, 150, 150, true, false },    // [4] BLOBS_LEARN
  { { HIGH,  LOW, false }, 800, 800, true, false },    // [5] MAPPING_LIB
  { { HIGH, HIGH, false }, 500, 500, true, false },    // [6] RAW_MATRIX
  { { HIGH, HIGH, false }, 1000, 1000, true, false },  // [7] INTERP_MATRIX
  { { HIGH, HIGH, false },  10,  10, true, false }     // [8] ERROR
};

e256_level_t e256_l[4] = {
  { { HIGH,  LOW, false },  1, 50, 12, false },  // [0]  SIG_IN     
  { {  LOW, HIGH, false },  1, 31, 17, false },  // [1]  SIG_OUT    
  { {  LOW,  LOW, false }, 13, 31, 29, false },  // [2]  LINE_OUT
  { { HIGH, HIGH, false },  2, 60,  3, false }   // [3]  THRESHOLD
};

e256_control_t e256_ctr = {
  &e256_e,    // encoder_ptr
  &e256_m[0], // modes_ptr
  &e256_l[0]  // levels_ptr
};

uint8_t playMode = BLOBS_PLAY;
//uint8_t playMode = RAW_MATRIX;
//uint8_t playMode = MAPPING_LIB;
uint8_t lastMode = NULL;
uint8_t levelMode = THRESHOLD;

uint32_t ledsTimeStamp = 0;
uint8_t ledsIterCount = 0;

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

void setup_leds(uint8_t mode){
  leds_t* leds = &e256_ctr.levels[mode].leds;
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
  digitalWrite(LED_PIN_D1, leds->D1);
  digitalWrite(LED_PIN_D2, leds->D2);
};

void set_mode(uint8_t mode) {
  ledsTimeStamp = millis();
  ledsIterCount = 0;
  lastMode = playMode;
  playMode = mode;
  e256_ctr.levels[levelMode].leds.update = false;
  setup_leds(mode);
  e256_ctr.modes[mode].leds.update = true;
  e256_ctr.modes[mode].run = true;
  #if defined(DEBUG_MODES)
    Serial.printf("\nSET_MODE:%d", mode);
  #endif
};

void set_level(uint8_t level, uint8_t value) {
  e256_ctr.encoder->write(value << 2);
  e256_ctr.modes[playMode].leds.update = false;
  setup_leds(level);
  e256_ctr.levels[level].run = true;
  #if defined(DEBUG_LEVELS)
    Serial.printf("\nSET_LEVEL:%d_%d", level, value);
  #endif
};

// Selec the current mode and perform the matrix sensor calibration 
inline void update_buttons(void) {
  BUTTON_L.update();
  BUTTON_R.update();
  // ACTION: BUTTON_L short press
  // FONCTION: CALIBRATE THE SENSOR MATRIX
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() < LONG_HOLD) {
    set_mode(CALIBRATE);
  };
  // ACTION: BUTTON_L long press
  // FONCTION: FLASH THE CONFIG FILE (config.json)
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() > LONG_HOLD) {
    set_mode(FLASH_CONFIG);
  };
  // ACTION: BUTTON_R long press
  // FONCTION_A: MIDI_PLAY (send all blob values over MIDI format)
  // FONCTION_B: MIDI_LEARN (send blob values separately for Max4Live MIDI_LEARN)
  // LEDs: blink alternately, slow for playing mode and fast or learning mode
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() > LONG_HOLD) {
    if (playMode == BLOBS_PLAY) {
      set_mode(BLOBS_LEARN);
    } else {
      set_mode(BLOBS_PLAY);
    };
  };
  // ACTION: BUTTON_R short press
  // FONCTION: SELECT_MODE
  // [2]-LINE_OUT
  // [3]-SIG_IN
  // [4]-SIG_OUT
  // [5]-THRESHOLD
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() < LONG_HOLD) {
    levelMode = (levelMode + 1) % 4; // Loop into level modes
    set_level(levelMode, e256_ctr.levels[levelMode].val);
  };
};

// Levels values adjustment using rotary encoder
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
      if (playMode == CALIBRATE) {
        if (ledsIterCount < CALIBRATE_ITER) {
          ledsTimeStamp = millis();
          ledsIterCount++;
        }
        else {
          e256_ctr.modes[mode].leds.update = false;
          playMode = lastMode;
        };
      }
      else {
        ledsTimeStamp = millis();
      };
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
    mapp_trigsParams[i].rect.Xmin = config[i]["Xmin"];
    mapp_trigsParams[i].rect.Xmax = config[i]["Xmax"];
    mapp_trigsParams[i].rect.Ymin = config[i]["Ymin"];
    mapp_trigsParams[i].rect.Ymax = config[i]["Ymax"];
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
    mapp_togsParams[i].rect.Xmin = config[i]["Xmin"];
    mapp_togsParams[i].rect.Xmax = config[i]["Xmax"];
    mapp_togsParams[i].rect.Ymin = config[i]["Ymin"];
    mapp_togsParams[i].rect.Ymax = config[i]["Ymax"];
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
    mapp_vSlidersParams[i].rect.Xmin = config[i]["Xmin"];
    mapp_vSlidersParams[i].rect.Xmax = config[i]["Xmax"];
    mapp_vSlidersParams[i].rect.Ymin = config[i]["Ymin"];
    mapp_vSlidersParams[i].rect.Ymax = config[i]["Ymax"];
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
    mapp_hSlidersParams[i].rect.Xmin = config[i]["Xmin"];
    mapp_hSlidersParams[i].rect.Xmax = config[i]["Xmax"];
    mapp_hSlidersParams[i].rect.Ymin = config[i]["Ymin"];
    mapp_hSlidersParams[i].rect.Ymax = config[i]["Ymax"];
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
    mapp_touchpadsParams[i].rect.Xmin = config[i]["Xmin"];
    mapp_touchpadsParams[i].rect.Xmax = config[i]["Xmax"];
    mapp_touchpadsParams[i].rect.Ymin = config[i]["Ymin"];
    mapp_touchpadsParams[i].rect.Ymax = config[i]["Ymax"];
    mapp_touchpadsParams[i].CCx = config[i]["CCx"];     // X axis MIDI cChange mapping
    mapp_touchpadsParams[i].CCy = config[i]["CCy"];     // X axis MIDI cChange mapping
    mapp_touchpadsParams[i].CCz = config[i]["CCz"];     // X axis MIDI cChange mapping
    mapp_touchpadsParams[i].CCs = config[i]["CCs"];     // XY size MIDI cChange mapping
    mapp_touchpadsParams[i].CCxyv = config[i]["CCxyv"]; // XY velocity MIDI cChange mapping
    mapp_touchpadsParams[i].CCzv = config[i]["CCzv"];   // XY velocity MIDI cChange mapping
 };
  return true;
};

inline bool config_load_mapping_polygons(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  };
  mapping_polygons_alloc(config.size());
  for (uint8_t i = 0; i < mapp_polygons; i++) {
    mapp_polygonsParams[i].vertices_cnt = config[i]["cnt"];
    for (uint8_t j = 0; j < config[i]["cnt"]; j++) {
      mapp_polygonsParams[i].vertices[j].x = (float)config[i]["vertrices"][j]["X"];
      mapp_polygonsParams[i].vertices[j].y = (float)config[i]["vertrices"][j]["Y"];
    };
  };
  return true;
};

bool config_load_mapping(const JsonObject &config) {
  if (config.isNull()) {
    return false;
  };
  if (!config_load_mapping_triggers(config["triggers"])) {
    return false;
  };
  if (!config_load_mapping_toggles(config["toggles"])) {
    return false;
  };
  if (!config_load_mapping_vSliders(config["vSliders"])) {
    return false;
  };
  if (!config_load_mapping_hSliders(config["hSliders"])) {
    return false;
  };
  if (!config_load_mapping_circles(config["circles"])) {
    return false;
  };
  if (!config_load_mapping_touchpads(config["touchpad"])) {
    return false;
  };
  if (!config_load_mapping_polygons(config["polygons"])) {
    return false;
  };
  return true;
};

void load_config(char* data_ptr) {
  StaticJsonDocument<2048> config;
  DeserializationError err = deserializeJson(config, data_ptr);
  if (err) {
    error(ERROR_WAITING_FOR_GONFIG);
  };
  if (!config_load_mapping(config["mapping"])) {
    error(ERROR_LOADING_GONFIG_FAILED);
  };
  set_mode(MAPPING_LIB);
};

inline void load_flash_config() {
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    error(ERROR_CONNECTING_FLASH);
    return;
  };
  SerialFlashFile configFile = SerialFlash.open("config.json");
  if (configFile) { // true if the file exists
    configSize = configFile.size();
    config_ptr = allocate(config_ptr, configSize);
    configFile.read(config_ptr, configSize);
    StaticJsonDocument<2048> config;
    DeserializationError err = deserializeJson(config, config_ptr);
    if (err) {
      // Load a default config file?
      error(ERROR_WAITING_FOR_GONFIG);
      return;
    };
    if (!config_load_mapping(config["mapping"])) {
      error(ERROR_LOADING_GONFIG_FAILED);
      return;
    };
    configFile.close();
    set_mode(MAPPING_LIB);
  }
  else {
    error(ERROR_NO_CONFIG_FILE);
  };
};

inline void flash_config(char* data_ptr, unsigned int size) {
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    error(ERROR_CONNECTING_FLASH);
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
      error(ERROR_WHILE_OPEN_FLASH_FILE);
      return;
    };
  }
  else {
    error(ERROR_FLASH_FULL);
    return;
  };
  if (size < FLASH_SIZE) {
    flashFile.write(data_ptr, size);
    flashFile.close();
  } else {
    error(ERROR_FILE_TO_BIG);
  };
};

void CONFIG_SETUP(void){
  setup_buttons();
  set_mode(CALIBRATE);
  //load_flash_config();
};

void update_config(void){
  update_buttons();
  update_encoder();
  update_leds();
  //flash_config(config_ptr, configLength);
};