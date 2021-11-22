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

// E256 LEDs CONSTANTES
#define LONG_HOLD                  1500

#define UPDATE_CONFIG_LED_TIMEON   50
#define UPDATE_CONFIG_LED_TIMEOFF  100
#define LOAD_CONFIG_LED_TIMEON     200
#define LOAD_CONFIG_LED_TIMEOFF    500 
#define ERROR_LED_TIMEON           200
#define ERROR_LED_TIMEOFF          500 

#define MIDI_PLAY_LED_TIMEON       600
#define MIDI_PLAY_LED_TIMEOFF      600
#define MIDI_LEARN_LED_TIMEON      100
#define MIDI_LEARN_LED_TIMEOFF     100
#define MIDI_MAPPING_LED_TIMEON    1000
#define MIDI_MAPPING_LED_TIMEOFF   100

#define CALIBRATE_LED_TIMEON        35
#define CALIBRATE_LED_TIMEOFF       100
#define CALIBRATE_LED_ITER          5

Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

Bounce BUTTON_L = Bounce();
Bounce BUTTON_R = Bounce();

preset_t presets[10] = {
  { 0, 0,  0,  0, false, false, false, false, NULL, NULL },  // [0] LOAD_CONFIG     ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 0, 0,  0,  0, false, false, false, false, NULL, NULL },  // [1] UPDATE_CONFIG   ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  {13, 31, 29, 0, false, false, false, false, LOW,  LOW  },  // [2] LINE_OUT        ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 1, 50, 12, 0, false, false, false, false, HIGH, LOW  },  // [3] SIG_IN          ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 1, 31, 17, 0, false, false, false, false, LOW,  HIGH },  // [4] SIG_OUT         ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 2, 60, 3,  0, false, false, false, false, HIGH, HIGH },  // [5] THRESHOLD       ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 0, 0,  0,  0, true,  false, false, false, HIGH, HIGH },  // [6] CALIBRATE       ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 0, 0,  0,  0, false, false, false, false, NULL, NULL },  // [7] MIDI_LEARN      ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 1, 7,  0,  0, false, false, false, false, NULL, NULL },  // [8] MIDI_PLAY       ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 0, 0,  0,  0, false, false, false, false, NULL, NULL }   // [9] MAPPING_LIB     ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
};

uint8_t currentMode = CALIBRATE;      // Init currentMode with CALIBRATE (SET as DEFAULT_MODE)
//uint8_t lastMode = LINE_OUT;        // Init lastMode with LINE_OUT (SET as DEFAULT_MODE)
//uint8_t lastMode = BLOBS_MAPPING;   // Init lastMode with MIDI_MAPPING (SET as DEFAULT_MODE)
uint8_t lastMode = MIDI_PLAY;        // Init lastMode with MIDI_MIDI_PLAY (SET as DEFAULT_MODE)

inline void setup_leds(void) {
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
  digitalWrite(LED_PIN_D1, LOW);
  digitalWrite(LED_PIN_D2, LOW);
};

// Hear it should not compile if you didn't install the library
// [Bounce2](https://github.com/thomasfredericks/Bounce2)
// https://www.pjrc.com/teensy/interrupts.html
// https://github.com/khoih-prog/Teensy_TimerInterrupt/blob/main/examples/SwitchDebounce/SwitchDebounce.ino

inline void setup_switches(void) {
  BUTTON_L.attach(BUTTON_PIN_L, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_R.attach(BUTTON_PIN_R, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_L.interval(25);                        // Debounce interval of 25 millis
  BUTTON_R.interval(25);                        // Debounce interval of 25 millis
};

// Selec the current mode and perform the matrix sensor calibration 
inline void update_presets_buttons(void) {
  BUTTON_L.update();
  BUTTON_R.update();
  // ACTION: BUTTON_L short press
  // FONCTION: CALIBRATE THE SENSOR MATRIX
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() < LONG_HOLD) {
    lastMode = currentMode; // keep track of last Mode to set it back after calibration
    currentMode = CALIBRATE;
    presets[CALIBRATE].update = true;
#if defined(DEBUG_BUTTONS)
    Serial.printf("\nBUTTON_L : CALIBRATE : %d", currentMode);
#endif
  };
  // ACTION: BUTTON_L long press
  // FONCTION: UPDATE FLASH CONFIG FILE
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() > LONG_HOLD) {
    lastMode = currentMode; // keep track of last Mode to set it back after saving
    currentMode = UPDATE_CONFIG;
    presets[UPDATE_CONFIG].update = true;
#if defined(DEBUG_BUTTONS)
    Serial.printf("\nBUTTON_L : UPDATE_CONFIG : %d", currentMode);
#endif
  };
  // ACTION: BUTTON_R short press
  // FONCTION: SELECT_MODE
  // [2]-LINE_OUT
  // [3]-SIG_IN
  // [4]-SIG_OUT
  // [5]-THRESHOLD
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() < LONG_HOLD) {
    currentMode = (currentMode + 3) % 6;   // Loop into the modes
    encoder.write(presets[currentMode].val << 2);
    presets[currentMode].setLed = true;
#if defined(DEBUG_BUTTONS)
    Serial.printf("\nBUTTON_R : SELECT_MODE : %d", currentMode);
#endif
  };
  // ACTION: BUTTON_R long press
  // FONCTION_A: MIDI_MIDI_PLAY (send all blob values over MIDI format)
  // FONCTION_B: MIDI_MIDI_LEARN (send blob values separately for Max4Live MIDI_LEARN)
  // LEDs: blink alternately, slow for playing mode and fast or learning mode
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() > LONG_HOLD) {
    if (currentMode == MIDI_PLAY) {
      currentMode = MIDI_LEARN;
      presets[MIDI_LEARN].update = true;
#if defined(DEBUG_BUTTONS)
      Serial.printf("\nMIDI_MIDI_PLAY : %d", currentMode);
#endif
    }
    else {
      currentMode = MIDI_PLAY;
      encoder.write(0x1);
      presets[MIDI_PLAY].update = true;
#if defined(DEBUG_BUTTONS)
      Serial.printf("\nMIDI_MIDI_LEARN : %d", currentMode);
#endif
    };
  };
};

// Values adjustment using rotary encoder
inline boolean setLevel(preset_t* preset_ptr) {
  uint8_t val = encoder.read() >> 2;
  if (val != preset_ptr->val) {
    if (val > preset_ptr->maxVal) {
      encoder.write(preset_ptr->maxVal << 2);
      return false;
    }
    else if (val < preset_ptr->minVal) {
      encoder.write(preset_ptr->minVal << 2);
      return false;
    }
    else {
      preset_ptr->val = val;
      return true;
    };
  }
  else {
    return false;
  };
};

// Update preset of each mode with the encoder position
inline void update_presets_encoder(void) {
  switch (currentMode) {
    case LINE_OUT:
      if (setLevel(&presets[LINE_OUT])) {
        presets[LINE_OUT].ledVal = map(presets[LINE_OUT].val, presets[LINE_OUT].minVal, presets[LINE_OUT].maxVal, 0, 255);
        presets[LINE_OUT].updateLed = true;
      };
      break;
    case SIG_IN:
      if (setLevel(&presets[SIG_IN])) {
        presets[SIG_IN].ledVal = map(presets[SIG_IN].val, presets[SIG_IN].minVal, presets[SIG_IN].maxVal, 0, 255);
        presets[SIG_IN].updateLed = true;
      };
      break;
    case SIG_OUT:
      if (setLevel(&presets[SIG_OUT])) {
        presets[SIG_OUT].ledVal = map(presets[SIG_OUT].val, presets[SIG_OUT].minVal, presets[SIG_OUT].maxVal, 0, 255);
        presets[SIG_OUT].updateLed = true;
      };
      break;
    case THRESHOLD:
      if (setLevel(&presets[THRESHOLD])) {
        presets[THRESHOLD].ledVal = map(presets[THRESHOLD].val, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal, 0, 255);
        interpThreshold = constrain(presets[THRESHOLD].val - 5, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
        presets[THRESHOLD].updateLed = true;
      };
      break;
    case MIDI_LEARN:
      if (setLevel(&presets[MIDI_LEARN]));
      break;
    default:
      break;
  };
};

inline void update_presets_usb_midi(void) {
  for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiIn); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
    switch (node_ptr->midiMsg.status) {
      case midi::NoteOn:
        // NOT_USED
        break;
      case midi::NoteOff:
        // NOT_USED
        break;
      case midi::ControlChange:
        switch (node_ptr->midiMsg.data1) {
          case LINE_OUT: // PROGRAM 0
            currentMode = LINE_OUT;
            presets[LINE_OUT].update = true;
            break;
          case SIG_IN: // PROGRAM 1
            currentMode = SIG_IN;
            presets[SIG_IN].update = true;
            break;
          case SIG_OUT: // PROGRAM 2
            currentMode = SIG_OUT;
            presets[SIG_OUT].update = true;
            break;
          case THRESHOLD: // PROGRAM 3
            currentMode = THRESHOLD;
            presets[THRESHOLD].val = map(node_ptr->midiMsg.data2, 0, 127, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
            encoder.write(presets[THRESHOLD].val << 2);
            interpThreshold = constrain(presets[THRESHOLD].val - 5, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
            presets[THRESHOLD].ledVal = map(node_ptr->midiMsg.data2, 0, 127, 0, 255);
            //presets[THRESHOLD].update = true;
            presets[THRESHOLD].setLed = true;
            presets[THRESHOLD].updateLed = true;
            break;
          case CALIBRATE: // PROGRAM 4
            lastMode = currentMode;
            currentMode = CALIBRATE;
            presets[CALIBRATE].update = true;
            break;
          case MIDI_PLAY: // PROGRAM 6
            currentMode = MIDI_PLAY;
            presets[MIDI_PLAY].update = true;
            presets[MIDI_PLAY].setLed = true;
            break;
          case MIDI_LEARN: // PROGRAM 7
            currentMode = MIDI_LEARN;
            presets[MIDI_LEARN].update = true;
            presets[MIDI_LEARN].setLed = true;
            break;
          case MAPPING_LIB: // PROGRAM 8
            currentMode = MAPPING_LIB;
            presets[MAPPING_LIB].update = true;
            presets[MAPPING_LIB].setLed = true;
            break;
          case RAW_MATRIX: // PROGRAM 9
            currentMode = RAW_MATRIX;
            //presets[RAW_MATRIX].updat = true;
            break;
          case INTERP_MATRIX: // PROGRAM 10
            currentMode = INTERP_MATRIX;
            //presets[INTERP_MATRIX].update = true;
            break;
          case ALL_OFF: // PROGRAM 11
            currentMode = ALL_OFF;
            //presets[ALL_OFF].update = true;
            break;
          default:
            break;
        };
        break;
      default:
        break;
    };
  };
  llist_save_nodes(&midi_node_stack, &midiIn); // Save/rescure all midiOut nodes
};

inline void update_presets_usb_serial(void) {
  uint8_t mode = 0;
  uint8_t val = 0;
  if (Serial.available() == 2) {
    mode = Serial.read();
    val = Serial.read();
  };
  switch (mode) {
    case LOAD_CONFIG: // MODE 0
      currentMode = LOAD_CONFIG;
      presets[LOAD_CONFIG].update = true;
      break;
    case UPDATE_CONFIG: // MODE 1
      currentMode = UPDATE_CONFIG;
      presets[UPDATE_CONFIG].update = true;
      break;
    case LINE_OUT: // MODE 2
      currentMode = LINE_OUT;
      presets[LINE_OUT].update = true;
      break;
    case SIG_IN: // MODE 3
      currentMode = SIG_IN;
      presets[SIG_IN].update = true;
      break;
    case SIG_OUT: // MODE 4
      currentMode = SIG_OUT;
      presets[SIG_OUT].update = true;
      break;
    case THRESHOLD: // MODE 5
      currentMode = THRESHOLD;
      presets[THRESHOLD].val = map(val, 0, 127, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
      encoder.write(presets[THRESHOLD].val << 2);
      interpThreshold = constrain(presets[THRESHOLD].val - 5, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
      presets[THRESHOLD].ledVal = map(val, 0, 127, 0, 255);
      //presets[THRESHOLD].update = true;
      presets[THRESHOLD].setLed = true;
      presets[THRESHOLD].updateLed = true;
      break;
    case CALIBRATE: // MODE 6
      lastMode = currentMode;
      currentMode = CALIBRATE;
      presets[CALIBRATE].update = true;
      break;
    case MIDI_LEARN: // MODE 7
      currentMode = MIDI_LEARN;
      presets[MIDI_LEARN].update = true;
      presets[MIDI_LEARN].setLed = true;
      break;
    case MIDI_PLAY: // MODE 8
      currentMode = MIDI_PLAY;
      presets[MIDI_PLAY].update = true;
      presets[MIDI_PLAY].setLed = true;
      break;
    case MAPPING_LIB: // MODE 9
      currentMode = MAPPING_LIB;
      presets[MAPPING_LIB].update = true;
      presets[MAPPING_LIB].setLed = true;
      break;
    case RAW_MATRIX: // MODE 13
      currentMode = RAW_MATRIX;
      //presets[RAW_MATRIX].updat = true;
      break;
    case INTERP_MATRIX: // MODE 14
      currentMode = INTERP_MATRIX;
      //presets[INTERP_MATRIX].update = true;
      break;
    case ALL_OFF: // MODE 15
      currentMode = ALL_OFF;
      //presets[ALL_OFF].update = true;
      break;
    default:
      break;
  };
};

inline void led_control(uint8_t mode, uint8_t timeOn, uint8_t timeOff, int8_t iter) {
  static uint32_t timeStamp = 0;
  static uint8_t iterCount = 0;

  if (presets[mode].setLed) {
    presets[mode].setLed = false;
    pinMode(LED_PIN_D1, OUTPUT);
    pinMode(LED_PIN_D2, OUTPUT);
    timeStamp = millis();
    iterCount = 0;
  };
  if (!presets[mode].allDone) {
    if (iterCount <= iter) {
      if (millis() - timeStamp < timeOn && presets[mode].updateLed == true ) {
        presets[mode].updateLed = false;
        digitalWrite(LED_PIN_D1, HIGH);
        digitalWrite(LED_PIN_D2, HIGH);
      }
      else if (millis() - timeStamp > timeOn && presets[mode].updateLed == false) {
        presets[mode].updateLed = true;
        digitalWrite(LED_PIN_D1, LOW);
        digitalWrite(LED_PIN_D2, LOW);
      }
      else if (millis() - timeStamp > timeOn + timeOff) {
        if (iter != -1) {
          if (iterCount < iter) {
            timeStamp = millis();
            iterCount++;
          }
          else {
            presets[mode].allDone = true;
            currentMode = lastMode;
            presets[currentMode].setLed = true;
          };
        }
        else {
          timeStamp = millis();
        };
      };
    };
  };
};

// Update LEDs according to the mode and rotary encoder values
inline void update_leds(void) {
  static uint32_t timeStamp = 0;
  static uint8_t iter = 0;

  switch (currentMode) {
    case LOAD_CONFIG: // LEDs : Both LEDs are blinking fast
      led_control(LOAD_CONFIG, LOAD_CONFIG_LED_TIMEON, LOAD_CONFIG_LED_TIMEOFF, -1);
      break;
    case UPDATE_CONFIG: // LEDs : Both LEDs are blinking very fast
      led_control(UPDATE_CONFIG, UPDATE_CONFIG_LED_TIMEON, UPDATE_CONFIG_LED_TIMEOFF, -1);
      break;
    case LINE_OUT: // Adjust both LEDs intensity according to output volume value
      if (presets[LINE_OUT].setLed) {
        presets[LINE_OUT].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets[LINE_OUT].D1);
        digitalWrite(LED_PIN_D2, presets[LINE_OUT].D2);
      };
      if (presets[LINE_OUT].updateLed) {
        presets[LINE_OUT].updateLed = false;
        analogWrite(LED_PIN_D1, presets[LINE_OUT].ledVal);
        analogWrite(LED_PIN_D2, abs(presets[LINE_OUT].ledVal - 255));
      };
      break;
    case SIG_IN: // Adjust both LEDs intensity according to the input volume value
      if (presets[SIG_IN].setLed) {
        presets[SIG_IN].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets[SIG_IN].D1);
        digitalWrite(LED_PIN_D2, presets[SIG_IN].D2);
      };
      if (presets[SIG_IN].updateLed) {
        presets[SIG_IN].updateLed = false;
        analogWrite(LED_PIN_D1, presets[SIG_IN].ledVal);
        analogWrite(LED_PIN_D2, abs(presets[SIG_IN].ledVal - 255));
      };
      break;
    case SIG_OUT: // Adjust both LEDs intensity according to the output volume value
      if (presets[SIG_OUT].setLed) {
        presets[SIG_OUT].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets[SIG_OUT].D1);
        digitalWrite(LED_PIN_D2, presets[SIG_OUT].D2);
      };
      if (presets[SIG_OUT].updateLed) {
        presets[SIG_OUT].updateLed = false;
        analogWrite(LED_PIN_D1, presets[SIG_OUT].ledVal);
        analogWrite(LED_PIN_D2, abs(presets[SIG_OUT].ledVal - 255));
      };
      break;
    case THRESHOLD: // Adjust both LEDs intensity according to the threshold value
      if (presets[THRESHOLD].setLed) {
        presets[THRESHOLD].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets[THRESHOLD].D1);
        digitalWrite(LED_PIN_D2, presets[THRESHOLD].D2);
      };
      if (presets[THRESHOLD].updateLed) {
        presets[THRESHOLD].updateLed = false;
        analogWrite(LED_PIN_D1, presets[THRESHOLD].ledVal);
        analogWrite(LED_PIN_D2, abs(presets[THRESHOLD].ledVal - 255));
      };
      break;
    case CALIBRATE: // LEDs : both LED are blinking three time
      led_control(CALIBRATE, CALIBRATE_LED_TIMEON, CALIBRATE_LED_TIMEOFF, CALIBRATE_LED_ITER);
      break;
    case MIDI_PLAY: // LEDs : blink alternately slow
      led_control(MIDI_PLAY, MIDI_PLAY_LED_TIMEON, MIDI_PLAY_LED_TIMEOFF, -1);
      break;
    case MIDI_LEARN: // LEDs : blink alternately fast
      led_control(MIDI_LEARN, MIDI_LEARN_LED_TIMEON, MIDI_LEARN_LED_TIMEOFF, -1);
      break;
    case MAPPING_LIB: // LEDs : blink alternately slow
      led_control(MAPPING_LIB, MIDI_MAPPING_LED_TIMEON, MIDI_MAPPING_LED_TIMEOFF, -1);
      break;
    default:
      break;
  };
};

void update_presets(){
#if defined(__MK20DX256__)  // Teensy 3.1 & 3.2
  update_presets_usb_midi();
#endif
#if defined(__IMXRT1062__)  // Teensy 4.0 & 4.1
  update_presets_buttons();
  update_presets_encoder();
  //update_presets_usb_midi();
  update_presets_usb_serial();
  update_leds();
#endif
};

inline void config_error(uint8_t status) {
  while (1) {
#if defined(DEBUG_SERIAL_FLASH)
    Serial.printf("\nDEBUG_SERIAL_FLASH\t%d", status);
#endif
    //digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(LED_PIN_D1, HIGH);
    digitalWrite(LED_PIN_D2, HIGH);
    delay(40);
    //digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_PIN_D1, LOW);
    digitalWrite(LED_PIN_D2, LOW);
    delay(40);
  };
};

uint32_t serialUpdateConfigTimeStamp = 0;

inline void flushError(void) {
  uint32_t lastReceiveTime = millis();
  char usbBuffer[USB_BUFFER_SIZE];
  // We assume the serial receive part is finished when we have not received something for 3 seconds
  while (Serial.available() || millis() < lastReceiveTime + 3000) {
    if (Serial.readBytes(usbBuffer, USB_BUFFER_SIZE)) {
      lastReceiveTime = millis();
      serialUpdateConfigTimeStamp = millis();
    };
  };
};

inline void usb_serial_update_config(void) {
  
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    config_error(0);
  };

  // Waiting for config file!
  while (millis() - serialUpdateConfigTimeStamp < SERIAL_UPDATE_CONFIG_TIMEOUT) {
    if (Serial.available()) {

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

      // Flash LED while formatting!
      while (!SerialFlash.ready()) {
#if defined(DEBUG_SERIAL_FLASH)
        Serial.printf("\nDEBUG_SERIAL_FLASH\t%d", 2);
#endif
        delay(100);
        digitalWrite(LED_PIN_D1, HIGH);
        digitalWrite(LED_PIN_D2, HIGH);
        delay(100);
        digitalWrite(LED_PIN_D1, LOW);
        digitalWrite(LED_PIN_D2, LOW);
      };

      digitalWrite(LED_PIN_D1, HIGH);
      digitalWrite(LED_PIN_D2, HIGH);
      
      // We assume the serial receive part is finished when we have not received something for 3 seconds
      while (Serial.available() || lastReceiveTime + 3000 > millis()) {

        uint16_t available = Serial.readBytes(usbBuffer, USB_BUFFER_SIZE);
        
        if (available) {
          lastReceiveTime = millis();
          serialUpdateConfigTimeStamp = millis();
        };

        for (uint16_t usbBufferIndex = 0; usbBufferIndex < available; usbBufferIndex++) {
          uint8_t b = usbBuffer[usbBufferIndex];

          ////////////////////////////////////////// STATE_START
          if (state == STATE_START) {
            // Start byte - Reepat start is fine
            if (b == BYTE_START) {
              for (uint8_t i = 0; i < FILENAME_STRING_SIZE; i++) {
                filename[i] = 0x00;
              };
              filenameIndex = 0;
            }
            // Valid characters are A-Z, 0-9, comma, period, colon, dash, underscore
            else if ((b >= 'A' && b <= 'Z') || (b >= '0' && b <= '9') || b == '.' || b == ',' || b == ':' || b == '-' || b == '_') {
              filename[filenameIndex++] = b;
              if (filenameIndex >= FILENAME_STRING_SIZE) {
                //Error name too long
                flushError();
                return;
              };
            }
            // Filename end character
            else if (b == BYTE_SEPARATOR) {
              if (filenameIndex == 0) {
                // Error empty filename
                flushError();
                return;
              };
              // Change state
              state = STATE_SIZE;
              fileSizeIndex = 0;
              fileSize = 0;
            }
            // Invalid character
            else {
              // Error bad filename
              flushError();
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
                SerialFlash.remove(filename);  // It doesn't reclaim the space, but it does let you create a new file with the same name
              };
              // Create a new file and open it for writing
              if (SerialFlash.create(filename, fileSize)) {
                flashFile = SerialFlash.open(filename);
                if (!flashFile) {
                  // Error flash file open
                  flushError();
                  return;
                };
              }
              else {
                // Error flash create (no room left?)
                flushError();
                return;
              };
            }
            else {
              // Error invalid length requested
              flushError();
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
              //Serial.println("esc");
              escape = 1;
            }
            // End of file
            else if (b == BYTE_START) {
              //Serial.println("End of file");
              state = STATE_START;
              flashFile.write(flashBuffer, flashBufferIndex);
              flashFile.close();
              flashBufferIndex = 0;
            }
            // Normal byte; add to buffer
            else {
              flashBuffer[flashBufferIndex++] = b;
            };
            // The buffer is filled; write to SD card
            if (flashBufferIndex >= FLASH_BUFFER_SIZE) {
              flashFile.write(flashBuffer, FLASH_BUFFER_SIZE);
              flashBufferIndex = 0;
            };
          };

        };
      };
      // Success!  Turn the LEDs off
      digitalWrite(LED_PIN_D1, LOW);
      digitalWrite(LED_PIN_D2, LOW);
    };
  };
};

//////////////////////////////////////// LOAD CONFIG
inline bool config_load_mapping_triggers(const JsonArray& config) {
  if (config.isNull()) {
    return false;
  }
  mapping_triggers_alloc(config.size());
  for (uint8_t i = 0; i < mapp_trigs; i++) {
    mapp_trigsParams[i].rect.Xmin = config[i]["Xmin"];
    mapp_trigsParams[i].rect.Xmax = config[i]["Xmax"];
    mapp_trigsParams[i].rect.Ymin = config[i]["Ymin"];
    mapp_trigsParams[i].rect.Ymax = config[i]["Ymax"];
    mapp_trigsParams[i].note = config[i]["note"];
  }
  return true;
}

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
    //mapp_circlesParams[i].CCradius = config[i]["CCradius"];
    //mapp_circlesParams[i].CCtheta = config[i]["CCtheta"];
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
  
  //pinMode(LED_BUILTIN, OUTPUT);

  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    config_error(1);
  };

  uint8_t configData[1024];
  SerialFlashFile configFile = SerialFlash.open("config.json");
  configFile.read(configData, configFile.size());

  StaticJsonDocument<1024> config;
  DeserializationError error = deserializeJson(config, configData);
  if (error) {
    // Waiting for config file!
    // Load the default config file!
    currentMode = UPDATE_CONFIG;
    usb_serial_update_config();
    //config_error(2);
  };

  if (!config_load_mapping(config["mapping"])) {
    // Loading JSON config failed!
    config_error(3);
  };

  configFile.close();
};

void CONFIG_SETUP(void){
  setup_leds();
  setup_switches();
  load_config();
};