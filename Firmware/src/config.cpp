/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "config.h"
#include "interp.h"
#include "midi_bus.h"
#include "mapping_lib.h"

#include <ArduinoJson.h>
#include <SerialFlash.h>
#include <Bounce2.h>                   // https://github.com/thomasfredericks/Bounce2
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>                   // https://github.com/PaulStoffregen/Encoder

#define LONG_HOLD 1500
#define CALIBRATE_ITER 10

// The modes below can be selected using E256 built-in switches
Bounce BUTTON_L = Bounce();
Bounce BUTTON_R = Bounce();
e256_mode_t modes[9] = {
  { { HIGH,  LOW, false, false }, 200, 500, true, false }, // [0] LOAD_CONFIG
  { { HIGH, HIGH, false, false }, 150, 900, true, false }, // [1] UPLOAD_CONFIG
  { { HIGH, HIGH, true,   true },  30,  30, true,  true }, // [2] CALIBRATE
  { { HIGH, LOW,  false, false }, 500, 500, true, false }, // [3] BLOBS_PLAY
  { { HIGH, LOW,  false, false }, 150, 150, true, false }, // [4] BLOBS_LEARN
  { { HIGH, LOW,  false, false }, 800, 800, true, false }, // [5] MAPPING_LIB
  { { HIGH, HIGH, false, false }, 500, 500, true, false }, // [6] RAW_MATRIX
  { { HIGH, HIGH, false, false }, 500, 500, true, false }, // [7] INTERP_MATRIX
  { { HIGH, HIGH, false, false },  10,  10, true, false }  // [8] ERROR
};  

// The levels below can be adjusted using E256 built-in encoder
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
e256_level_t levels[4] = {
  { { HIGH,  LOW, false, false },  1, 50, 12, false },  // [0]  SIG_IN     
  { {  LOW, HIGH, false, false },  1, 31, 17, false },  // [1]  SIG_OUT    
  { {  LOW,  LOW, false, false }, 13, 31, 29, false },  // [2]  LINE_OUT
  { { HIGH, HIGH, false, false },  2, 60,  3, false }   // [3]  THRESHOLD
};

uint8_t currentMode = CALIBRATE;
uint8_t currentLevel = THRESHOLD;

uint8_t lastMode = BLOBS_PLAY;

uint32_t ledsTimeStamp = 0;
uint8_t ledsIterCount = 0;

// Her it should not compile if you didn't install the library
// [Bounce2]: https://github.com/thomasfredericks/Bounce2
// https://www.pjrc.com/teensy/interrupts.html
// https://github.com/khoih-prog/Teensy_TimerInterrupt/blob/main/examples/SwitchDebounce/SwitchDebounce.ino
inline void encoder_hardware_setup(void) {
  BUTTON_L.attach(BUTTON_PIN_L, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_R.attach(BUTTON_PIN_R, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_L.interval(25);                        // Debounce interval of 25 millis
  BUTTON_R.interval(25);                        // Debounce interval of 25 millis
};

inline void leds_hardware_setup(void) {
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
  digitalWrite(LED_PIN_D1, LOW);
  digitalWrite(LED_PIN_D2, LOW);
};

// Selec the current mode and perform the matrix sensor calibration 
inline void update_buttons(void) {
  BUTTON_L.update();
  BUTTON_R.update();
  // ACTION: BUTTON_L short press
  // FONCTION: CALIBRATE THE SENSOR MATRIX
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() < LONG_HOLD) {
    lastMode = currentMode; // keep track of last Mode to set it back after calibration
    currentMode = CALIBRATE;
    modes[currentMode].leds.setup = true;
    modes[currentMode].leds.update = true;
    modes[currentMode].run = true;
    ledsTimeStamp = millis();
    #if defined(DEBUG_BUTTONS)
    Serial.printf("\nDEBUG_BUTTONS\tBUTTON_L_CALIBRATE:%d", currentMode);
    #endif
  };
  // ACTION: BUTTON_L long press
  // FONCTION: UPLOAD NEW CONFIG FILE (config.json)
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() > LONG_HOLD) {
    lastMode = currentMode; // keep track of last Mode to set it back after saving
    currentMode = UPLOAD_CONFIG;
    modes[currentMode].leds.setup = true;
    modes[currentMode].leds.update = true;
    modes[currentMode].run = true;
    ledsTimeStamp = millis();
    #if defined(DEBUG_BUTTONS)
    Serial.printf("\nDEBUG_BUTTONS\tBUTTON_L_UPDATE_CONFIG:%d", currentMode);
    #endif
  };
  // ACTION: BUTTON_R long press
  // FONCTION_A: MIDI_PLAY (send all blob values over MIDI format)
  // FONCTION_B: MIDI_LEARN (send blob values separately for Max4Live MIDI_LEARN)
  // LEDs: blink alternately, slow for playing mode and fast or learning mode
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() > LONG_HOLD) {
    if (currentMode == BLOBS_PLAY) {
      currentMode = BLOBS_LEARN;
      modes[currentMode].leds.setup = true;
      modes[currentMode].leds.update = true;
      modes[currentMode].run = true;
      encoder.write(0x1);
      ledsTimeStamp = millis();
      #if defined(DEBUG_BUTTONS)
      Serial.printf("\nDEBUG_BUTTONS\tBUTTON_R_BLOBS_LEARN:%d", currentMode);
      #endif
    }
    else {
      currentMode = BLOBS_PLAY;
      modes[currentMode].leds.setup = true;
      modes[currentMode].leds.update = true;
      modes[currentMode].run = true;
      ledsTimeStamp = millis();
      #if defined(DEBUG_BUTTONS)
      Serial.printf("\nDEBUG_BUTTONS\tBUTTON_R_BLOBS_PLAY:%d", currentMode);
      #endif
    };
  };
  // ACTION: BUTTON_R short press
  // FONCTION: SELECT_MODE
  // [2]-LINE_OUT
  // [3]-SIG_IN
  // [4]-SIG_OUT
  // [5]-THRESHOLD
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() < LONG_HOLD) {
    modes[currentMode].leds.update = false;
    currentLevel = (currentLevel+1) % 4;   // Loop into the levels
    levels[currentLevel].leds.setup = true;
    levels[currentLevel].run = true;
    encoder.write(levels[currentLevel].val << 2);
    #if defined(DEBUG_BUTTONS)
    Serial.printf("\nDEBUG_BUTTONS\tBUTTON_R_SELECT_MODE:%d", currentLevel);
    #endif
  };
};

// Values adjustment using rotary encoder
inline boolean setLevel(e256_level_t* level_ptr) {
  uint8_t val = encoder.read() >> 2;
  if (val != level_ptr->val) {
    if (val > level_ptr->maxVal) {
      encoder.write(level_ptr->maxVal << 2);
      return false;
    }
    else if (val < level_ptr->minVal) {
      encoder.write(level_ptr->minVal << 2);
      return false;
    }
    else {
      level_ptr->val = val;
      return true;
    };
  }
  else {
    return false;
  };
};

// Update levels [val] of each mode using the rotary encoder
inline void update_encoder(void) {
  if (setLevel(&levels[currentLevel])) {
    levels[currentLevel].leds.update = true;
  };
};

// Generic function to update mmodes 
inline void set_mode(e256_level_t* levels_ptr, e256_mode_t* modes_ptr) {
  levels_ptr->leds.update = false;
  modes_ptr->leds.setup = true;
  modes_ptr->leds.update = true;
  modes_ptr->run = true;
};

// Generic function to update levels 
inline void set_level(e256_mode_t* modes_ptr, e256_level_t* levels_ptr, uint8_t value) {
  modes_ptr->leds.update = false;
  levels_ptr->val = constrain(value, levels_ptr->minVal, levels_ptr->maxVal);
  encoder.write(levels_ptr->val << 2);
  levels_ptr->leds.setup = true;
  levels_ptr->leds.update = true;
  levels_ptr->run = true;
};

inline void usb_set_params(void) {
  for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiIn); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
    if (node_ptr->midiMsg.status == midi::ControlChange) {
      currentLevel = node_ptr->midiMsg.data1;
      set_level(&modes[currentMode], &levels[node_ptr->midiMsg.data1], node_ptr->midiMsg.data2);
      ledsTimeStamp = millis();
    }
    else if (node_ptr->midiMsg.status == midi::ProgramChange) {
      currentMode = node_ptr->midiMsg.data1;
      set_mode(&levels[currentLevel], &modes[node_ptr->midiMsg.data1]);
    };
    llist_save_nodes(&midi_node_stack, &midiIn); // Save/rescure all midiOut nodes
  };
};

inline void leds_setup(void* struct_ptr){
  leds_t* leds = (leds_t*)struct_ptr;
  if (leds->setup) {
    leds->setup = false;
    pinMode(LED_PIN_D1, OUTPUT);
    pinMode(LED_PIN_D2, OUTPUT);
    digitalWrite(LED_PIN_D1, leds->D1);
    digitalWrite(LED_PIN_D2, leds->D2);
  };
};

inline void blink_leds(e256_mode_t* mode_ptr) {

    leds_setup(mode_ptr);

  if (mode_ptr->leds.update) {
    if (millis() - ledsTimeStamp < mode_ptr->timeOn && mode_ptr->toggle == true ) {
      mode_ptr->toggle = false;
      digitalWrite(LED_PIN_D1, mode_ptr->leds.D1);
      digitalWrite(LED_PIN_D2, mode_ptr->leds.D2);
    }
    else if (millis() - ledsTimeStamp > mode_ptr->timeOn && mode_ptr->toggle == false) {
      mode_ptr->toggle = true;
      digitalWrite(LED_PIN_D1, !mode_ptr->leds.D1);
      digitalWrite(LED_PIN_D2, !mode_ptr->leds.D2);
    }
    else if (millis() - ledsTimeStamp > mode_ptr->timeOn + mode_ptr->timeOff) {
      if (currentMode == CALIBRATE) {
        if (ledsIterCount < CALIBRATE_ITER) {
          ledsTimeStamp = millis();
          ledsIterCount++;
        }
        else {
          ledsIterCount = 0;
          mode_ptr->leds.update = false;
          currentMode = lastMode;
        };
      }
      else {
        mode_ptr->toggle = true;
        ledsTimeStamp = millis();
      };
    };
  };
};

inline void fade_leds(e256_level_t* levels_ptr) {
  
    leds_setup(levels_ptr);

  if (levels_ptr->leds.update) {
    levels_ptr->leds.update = false;
    uint8_t ledVal = constrain(map(levels_ptr->val, levels_ptr->minVal, levels_ptr->maxVal, 0, 255), 0, 255);
    analogWrite(LED_PIN_D1, abs(255 - ledVal));
    analogWrite(LED_PIN_D2, ledVal);
  };
};

// Update LEDs according to the mode and rotary encoder values
inline void update_leds(void) {
  fade_leds(&levels[currentLevel]);
  blink_leds(&modes[currentMode]);
};


//////////////////////////////////////// WRITE CONFIG
//Using: /Synth/Software/Python/usb_config/rawfile-uploader.py
// $ python rawfile-uploader.py <port> config.json
#define FLASH_CHIP_SELECT     6
// Buffer sizes
#define USB_BUFFER_SIZE       128
#define FLASH_BUFFER_SIZE     4096
// Max filename length (8.3 plus a null char terminator)
#define FILENAME_STRING_SIZE  14
// State machine
#define STATE_START           0
#define STATE_SIZE            1
#define STATE_CONTENT         2
// Special bytes in the communication protocol
#define HAND_SHAKE            0x7f
#define BYTE_START            0x7e
#define BYTE_ESCAPE           0x7d
#define BYTE_SEPARATOR        0x7c

inline void flushError(void) {
  uint32_t lastReceiveTime = millis();
  char usbBuffer[USB_BUFFER_SIZE];
  // We assume the serial receive part is finished when we have not received something for 3 seconds
  while (Serial.available() || millis() < lastReceiveTime + 3000) {
    if (Serial.readBytes(usbBuffer, USB_BUFFER_SIZE)) {
      lastReceiveTime = millis();
    };
  };
};

inline void usb_serial_upload_config(void) {

  if (modes[UPLOAD_CONFIG].run == true){
    modes[UPLOAD_CONFIG].run = false;

    if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
      currentMode = ERROR;
      modes[currentMode].leds.setup = true;
      modes[currentMode].leds.update = true;
      #if defined(DEBUG_SERIAL_FLASH)
      Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_CONNECTING_FLASH");
      #endif
      return;
    };

    // Flash LED when flash is ready
    while (!SerialFlash.ready());
    digitalWrite(LED_PIN_D1, HIGH);
    digitalWrite(LED_PIN_D2, HIGH);
    // Waiting for config file!
    while (!Serial.available());
    //Serial.read() == HAND_SHAKE

    SerialFlashFile flashFile;
    uint8_t state = STATE_START;
    uint8_t escape = 0;
    uint8_t fileSizeIndex = 0;
    uint32_t fileSize = 0;

    char usbBuffer[USB_BUFFER_SIZE];
    uint8_t flashBuffer[FLASH_BUFFER_SIZE];
    char filename[FILENAME_STRING_SIZE];

    uint16_t flashBufferIndex = 0;
    uint8_t filenameIndex = 0;
    uint32_t lastReceiveTime = millis();

    // We assume the serial receive part is finished when we have not received something for 3 seconds
    while (Serial.available() && (millis() - lastReceiveTime) < 3000) {

      uint16_t available = Serial.readBytes(usbBuffer, USB_BUFFER_SIZE);
      if (available) lastReceiveTime = millis();
    
      for (uint16_t index = 0; index < available; index++) {
        uint8_t b = usbBuffer[index];
        ////////////////////////////////////////// STATE_START
        if (state == STATE_START) {
          // Start byte - Reepat start is fine
          if (b == BYTE_START) {
            memset((char*)filename, 0, FILENAME_STRING_SIZE);
            filenameIndex = 0;
          }
          // Valid characters are a-z, 0-9, point
          else if ((b >= 'a' && b <= 'z') || (b >= '0' && b <= '9') || b == '.') {
            filename[filenameIndex++] = b;
            if (filenameIndex >= FILENAME_STRING_SIZE) {
              flushError();
              currentMode = ERROR;
              modes[currentMode].leds.setup = true;
              modes[currentMode].leds.update = true;
              #if defined(DEBUG_SERIAL_FLASH)
              Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_NAME_TO_LONG!");
              #endif
              return;
            };
          }
          // Filename end character
          else if (b == BYTE_SEPARATOR) {
            if (filenameIndex == 0) {
              flushError();
              currentMode = ERROR;
              modes[currentMode].leds.setup = true;
              modes[currentMode].leds.update = true;
              #if defined(DEBUG_SERIAL_FLASH)
              Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_EMPTY_FILENAME!");
              #endif
              return;
            };
            // Change state
            state = STATE_SIZE;
            fileSizeIndex = 0;
            fileSize = 0;
          }
          else {
            flushError();
            currentMode = ERROR;
            modes[currentMode].leds.setup = true;
            modes[currentMode].leds.update = true;
            #if defined(DEBUG_SERIAL_FLASH)
            Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_INVALID_FILENAME_CHARACTER!");
            #endif
            return;
          };
        }
        ////////////////////////////////////////// STATE_SIZE
        // We read 4 bytes as a uint32_t for file size
        else if (state == STATE_SIZE) {
          if (fileSizeIndex < 4) {
            fileSize = (fileSize << 8) + b;
            fileSizeIndex++;
          }
          else if (b == BYTE_SEPARATOR) {
            state = STATE_CONTENT;
            flashBufferIndex = 0;
            escape = 0;
            if (SerialFlash.exists(filename)) {
              SerialFlash.remove(filename); // It doesn't reclaim the space, but it does let you create a new file with the same name
            };
            // Create a new file and open it for writing
            if (SerialFlash.create(filename, fileSize)) {
              flashFile = SerialFlash.open(filename);
              if (!flashFile) {
                flushError();
                currentMode = ERROR;
                modes[currentMode].leds.setup = true;
                modes[currentMode].leds.update = true;
                #if defined(DEBUG_SERIAL_FLASH)
                Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_FLASH_FILE_OPEN!");
                #endif
                return;
              };
            }
            else {
              flushError();
              currentMode = ERROR;
              modes[currentMode].leds.setup = true;
              modes[currentMode].leds.update = true;
              #if defined(DEBUG_SERIAL_FLASH)
              Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_FLASH_FULL!");
              #endif
              return;
            };
          }
          else {
            // Error invalid length requested
            flushError();
            currentMode = ERROR;
            modes[currentMode].leds.setup = true;
            modes[currentMode].leds.update = true;
            #if defined(DEBUG_SERIAL_FLASH)
            Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_FLASH_FULL!");
            #endif
            return;
          };
        }
        ////////////////////////////////////////// STATE_CONTENT
        else if (state == STATE_CONTENT) {
          // Previous byte was escaped; unescape and add to buffer
          if (escape) {
            escape = 0;
            flashBuffer[flashBufferIndex++] = b ^ 0x20;
          }
          // Escape the next byte
          else if (b == BYTE_ESCAPE) {
            escape = 1;
          }
          // End of file
          else if (b == BYTE_START) {
            state = STATE_START;
            flashFile.write(flashBuffer, flashBufferIndex);
            flashFile.close();
            flashBufferIndex = 0;
          }
          // Normal byte; add to buffer
          else {
            flashBuffer[flashBufferIndex++] = b;
          };
          // The buffer is filled; write to Flash
          if (flashBufferIndex >= FLASH_BUFFER_SIZE) {
            flashFile.write(flashBuffer, FLASH_BUFFER_SIZE);
            flashBufferIndex = 0;
          };
        };
      };
    };
    // Success!  Turn the LEDs off
    //digitalWrite(LED_PIN_D1, LOW);
    //digitalWrite(LED_PIN_D2, LOW);
    //currentMode = lastMode;
    //_reboot_Teensyduino_();
  };
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

inline bool config_load_mapping(const JsonObject& config) {
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

inline void load_config(void) {
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    currentMode = ERROR;
    modes[currentMode].leds.setup = true;
    modes[currentMode].leds.update = true;
    #if defined(DEBUG_SERIAL_FLASH)
    Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_CONNECTING_FLASH");
    #endif
    return;
  };
  uint8_t configData[2048];
  SerialFlashFile configFile = SerialFlash.open("config.json");

  if (configFile) {  // true if the file exists
    configFile.read(configData, configFile.size());
    StaticJsonDocument<2048> config;
    DeserializationError err = deserializeJson(config, configData);
    if (err) {
      // Load a default config file?
      currentMode = ERROR;
      modes[currentMode].leds.setup = true;
      modes[currentMode].leds.update = true;
      #if defined(DEBUG_SERIAL_FLASH)
      Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_WAITING_FOR_GONFIG!\t%s", err.f_str());
      #endif
      return;
    };
    if (!config_load_mapping(config["mapping"])) {
      currentMode = ERROR;
      modes[currentMode].leds.setup = true;
      modes[currentMode].leds.update = true;
      #if defined(DEBUG_SERIAL_FLASH)
      Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_LOADING_GONFIG_FAILED!");
      #endif
      return;
    };
    configFile.close();
    modes[currentMode].leds.setup = true;
    modes[currentMode].leds.update = true;
    modes[currentMode].run = true;
  }
  else{
      currentMode = ERROR;
      modes[currentMode].leds.setup = true;
      modes[currentMode].leds.update = true;
      #if defined(DEBUG_SERIAL_FLASH)
      Serial.printf("\nDEBUG_SERIAL_FLASH\tERROR_NO_CONFIG_FILE!");
      #endif
  };
};

void CONFIG_SETUP(void){
  leds_hardware_setup();
  encoder_hardware_setup();
  load_config();
};

void update_config(){
  update_buttons();
  update_encoder();
  usb_set_params();
  update_leds();
  usb_serial_upload_config();
};