/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "presets.h"

// HARDWARE CONSTANTES **DO NOT CHANGE**
#define LED_PIN_D1              5
#define LED_PIN_D2              4
#define BUTTON_PIN_L            2
#define BUTTON_PIN_R            3
#define ENCODER_PIN_A           22
#define ENCODER_PIN_B           9

// SOFTWARE CONSTANTES **DO NOT CHANGE**
#define LONG_HOLD               1500
#define MIDI_BLOBS_LED_TIMEON   600
#define MIDI_BLOBS_LED_TIMEOFF  600
#define CALIBRATE_LED_TIMEON    35
#define CALIBRATE_LED_TIMEOFF   100
#define CALIBRATE_LED_ITER      4
#define SAVE_LED_TIMEON         20
#define SAVE_LED_TIMEOFF        50
#define SAVE_LED_ITER           10

Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
//Button BUTTON_L = Button(); // DEPRECATED
Bounce2::Button BUTTON_L = Bounce2::Button();
//Button BUTTON_R = Button(); // DEPRECATED
Bounce2::Button BUTTON_R = Bounce2::Button();

uint8_t currentMode = CALIBRATE;   // Init currentMode with CALIBRATE (DEFAULT_MODE)
uint8_t lastMode = LINE_OUT;       // Init lastMode with LINE_OUT (DEFAULT_MODE)

preset_t presets[7] = {
  {13, 31, 29, 0, false, false, false, LOW,  LOW },  // LINE_OUT   - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 50, 12, 0, false, false, false, HIGH, LOW },  // SIG_IN     - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 31, 17, 0, false, false, false, LOW,  HIGH},  // SIG_OUT    - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 5, 30, 10, 0, false, false, false, HIGH, HIGH},  // THRESHOLD  - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 1, 6,  1,  0, false, false, false, NULL, NULL},  // MIDI_LEARN - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 0, 0,  0,  0, true,  true,  false, NULL, NULL},  // CALIBRATE  - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
  { 0, 0,  0,  0, false, false, false, NULL, NULL}   // SAVE       - ARGS[minVal, maxVal, val, ledVal, setLed, updateLed, update, D1, D2]
};

void LEDS_SETUP(void) {
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
};

// Hear it should not compile if you didn't install the library (Manually!)
// [Bounce2](https://github.com/thomasfredericks/Bounce2)
// in your /Applications/Arduino/library
void SWITCHES_SETUP(void) {
  BUTTON_L.attach(BUTTON_PIN_L, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_R.attach(BUTTON_PIN_R, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_L.interval(25);                        // Debounce interval of 25 millis
  BUTTON_R.interval(25);                        // Debounce interval of 25 millis
};

void update_buttons(void) {
  BUTTON_L.update();
  BUTTON_R.update();
  // ACTION : BUTTON_L short press
  // FONCTION : CALIBRATE THE SENSOR MATRIX
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() < LONG_HOLD) {
    lastMode = currentMode;
    currentMode = CALIBRATE;
    presets[CALIBRATE].setLed = true;
    presets[CALIBRATE].updateLed = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_L : CALIBRATE : %d", currentMode);
#endif
  };
  // ACTION : BUTTON_L long press
  // FONCTION : SAVE ALL PARAMETERS TO THE EEPROM MEMORY
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() > LONG_HOLD) {
    lastMode = currentMode;
    currentMode = SAVE;
    presets[SAVE].setLed = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_L : SAVE : %d", currentMode);
#endif
  };
  // ACTION : BUTTON_R short press
  // FONCTION : SELECT_MODE
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() < LONG_HOLD) {
    lastMode = currentMode;                // Save last Mode
    currentMode = (currentMode + 1) % 4;   // Loop into the modes
    encoder.write(presets[currentMode].val << 2);
    presets[currentMode].setLed = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_R : SELECT_MODE : %d", currentMode);
#endif
  };
  // ACTION : BUTTON_R long press
  // FONCTION : MIDI_LEARN (send blob values separately for Max4Live MIDI_LEARN)
  // LEDs : alternates durring all the learning process
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() > LONG_HOLD) {
    lastMode = currentMode;
    currentMode = MIDI_BLOBS;
    encoder.write(0x1);
    presets[MIDI_BLOBS].setLed = true;
#if DEBUG_BUTTONS
    Serial.printf("\nMIDI_LEARN : %d", currentMode);
#endif
  };
};

void update_presets(void) {
  switch (currentMode) {
    case LINE_OUT:
      if (setLevel(&presets[LINE_OUT])) {
        presets[LINE_OUT].ledVal = map(presets[LINE_OUT].val, presets[LINE_OUT].minVal, presets[LINE_OUT].maxVal, 0, 255);
        presets[LINE_OUT].updateLed = true;
        presets[LINE_OUT].update = true;
      };
      break;
    case SIG_IN:
      if (setLevel(&presets[SIG_IN])) {
        presets[SIG_IN].ledVal = map(presets[SIG_IN].val, presets[SIG_IN].minVal, presets[SIG_IN].maxVal, 0, 255);
        presets[SIG_IN].updateLed = true;
        presets[SIG_IN].update = true;
      };
      break;
    case SIG_OUT:
      if (setLevel(&presets[SIG_OUT])) {
        presets[SIG_OUT].ledVal = map(presets[SIG_OUT].val, presets[SIG_OUT].minVal, presets[SIG_OUT].maxVal, 0, 255);
        presets[SIG_OUT].updateLed = true;
        presets[SIG_OUT].update = true;
      };
      break;
    case THRESHOLD:
      if (setLevel(&presets[THRESHOLD])) {
        presets[THRESHOLD].ledVal = map(presets[THRESHOLD].val, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal, 0, 255);
        interpThreshold = constrain(presets[THRESHOLD].val - 5, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
        presets[THRESHOLD].updateLed = true;
        presets[THRESHOLD].update = true;
      };
      break;
    case MIDI_BLOBS:
      if (setLevel(&presets[MIDI_BLOBS])) {
        presets[MIDI_BLOBS].updateLed = true;
        presets[MIDI_BLOBS].update = true;
      };
      break;
    default:
      break;
  };
};

// Update LEDs according to the mode and rotary encoder values
void update_leds(void) {
  static uint32_t timeStamp = 0;
  static uint8_t iter = 0;

  switch (currentMode) {
    case LINE_OUT:
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
    case SIG_IN:
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
    case SIG_OUT:
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
    case THRESHOLD:
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
    case MIDI_BLOBS: // LEDs : alternating blink
      if (presets[MIDI_BLOBS].setLed) {
        presets[MIDI_BLOBS].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
      };
      if (millis() - timeStamp < MIDI_BLOBS_LED_TIMEON && presets[MIDI_BLOBS].updateLed == true) {
        presets[MIDI_BLOBS].updateLed = false;
        digitalWrite(LED_PIN_D1, HIGH);
        digitalWrite(LED_PIN_D2, LOW);
      }
      else if (millis() - timeStamp > MIDI_BLOBS_LED_TIMEON && presets[MIDI_BLOBS].updateLed == false) {
        presets[MIDI_BLOBS].updateLed = true;
        digitalWrite(LED_PIN_D1, LOW);
        digitalWrite(LED_PIN_D2, HIGH);
      }
      else if (millis() - timeStamp > MIDI_BLOBS_LED_TIMEON + MIDI_BLOBS_LED_TIMEOFF) {
        timeStamp = millis();
      };
      break;
    case CALIBRATE: // LEDs : both LED are blinking three time
      if (presets[CALIBRATE].setLed) {
        presets[CALIBRATE].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
        iter = 0;
      };
      if (iter < CALIBRATE_LED_ITER) {
        if (millis() - timeStamp < CALIBRATE_LED_TIMEON && presets[CALIBRATE].updateLed == true) {
          presets[CALIBRATE].updateLed = false;
          digitalWrite(LED_PIN_D1, HIGH);
          digitalWrite(LED_PIN_D2, HIGH);
        }
        else if (millis() - timeStamp > CALIBRATE_LED_TIMEON && presets[CALIBRATE].updateLed == false) {
          presets[CALIBRATE].updateLed = true;
          digitalWrite(LED_PIN_D1, LOW);
          digitalWrite(LED_PIN_D2, LOW);
        }
        else if (millis() - timeStamp > CALIBRATE_LED_TIMEON + CALIBRATE_LED_TIMEOFF) {
          if (iter <= CALIBRATE_LED_ITER) {
            timeStamp = millis();
            iter++;
          }
          else {
            presets[CALIBRATE].update = true;
          };
        };
      };
      break;
    case SAVE: // LEDs : Both LED are blinking weary fast
      if (presets[SAVE].setLed) {
        presets[SAVE].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
        iter = 0;
      };
      if (iter < SAVE_LED_ITER) {
        if (millis() - timeStamp < SAVE_LED_TIMEON && presets[SAVE].updateLed == true) {
          presets[SAVE].updateLed = false;
          digitalWrite(LED_PIN_D1, HIGH);
          digitalWrite(LED_PIN_D2, HIGH);
        }
        else if (millis() - timeStamp > SAVE_LED_TIMEON && presets[SAVE].updateLed == false) {
          presets[SAVE].updateLed = true;
          digitalWrite(LED_PIN_D1, LOW);
          digitalWrite(LED_PIN_D2, LOW);
        }
        else if (millis() - timeStamp > SAVE_LED_TIMEON + SAVE_LED_TIMEOFF) {
          if (iter < SAVE_LED_ITER) {
            timeStamp = millis();
            iter++;
          };
        };
      };
      break;
    default:
      break;
  }
}

// Values adjustment using rotary encoder
boolean setLevel(preset_t* preset_ptr) {
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

// TODO
void preset_load(void) {
  for (int i = 0; i < 4; i++) {
    // EEPROM.read(i, preset[i].val); // uint8_t
  };
};

// TODO
void save_presets(void) {
  for (int i = 0; i < 4; i++) {
    //EEPROM.write(i, preset[i].val); // uint8_t
  };
};
