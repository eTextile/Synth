/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "presets.h"

void SETUP_LEDS(void) {
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
}

// Hear it should not compile if you didn't install the library (Manually!)
// [Bounce2](https://github.com/thomasfredericks/Bounce2)
// in your /Applications/Arduino/library
void SETUP_SWITCHES(Button* buttonA_ptr, Button* buttonB_ptr) {
  buttonA_ptr->attach(BUTTON_PIN_L, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  buttonB_ptr->attach(BUTTON_PIN_R, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  buttonA_ptr->interval(15);                        // Debounce interval of 15 millis
  buttonB_ptr->interval(15);                        // Debounce interval of 15 millis
}

void update_buttons(
  Button* buttonA_ptr,
  Button* buttonB_ptr,
  Encoder* encoder_ptr,
  preset_t* presets_ptr,
  uint8_t* curentMode_ptr,
  uint8_t* lastMode_ptr
) {

  buttonA_ptr->update();
  buttonB_ptr->update();

  // ACTION : BUTTON_L short press
  // FONCTION : CALIBRATE THE SENSOR MATRIX
  if (buttonA_ptr->rose() && buttonA_ptr->previousDuration() < LONG_HOLD) {
    pinMode(LED_PIN_D1, OUTPUT);
    pinMode(LED_PIN_D2, OUTPUT);
    *lastMode_ptr = *curentMode_ptr;
    *curentMode_ptr = CALIBRATE;
    presets_ptr[CALIBRATE].setLed = true;
    presets_ptr[CALIBRATE].update = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_L : CALIBRATE : %d", *curentMode_ptr);
#endif
  }

  // ACTION : BUTTON_L long press
  // FONCTION : SAVE ALL PARAMETERS TO THE EEPROM MEMORY
  if (buttonA_ptr->rose() && buttonA_ptr->previousDuration() > LONG_HOLD) {
    pinMode(LED_PIN_D1, OUTPUT);
    pinMode(LED_PIN_D2, OUTPUT);
    *lastMode_ptr = *curentMode_ptr;
    *curentMode_ptr = SAVE;
    presets_ptr[SAVE].setLed = true;
    presets_ptr[SAVE].update = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_L : SAVE : %d", *curentMode_ptr);
#endif
  }

  // ACTION : BUTTON_R short press
  // FONCTION : SELECT_MODE
  if (buttonB_ptr->rose() && buttonB_ptr->previousDuration() < LONG_HOLD) {
    *lastMode_ptr = *curentMode_ptr;                             // Save last Mode
    *curentMode_ptr = (*curentMode_ptr + 1) % 4;                 // Loop into the modes
    encoder_ptr->write(presets_ptr[*curentMode_ptr].val << 2);
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_R : SELECT_MODE : %d", *curentMode_ptr);
#endif
  }

  // ACTION : BUTTON_R long press
  // FONCTION : MIDI_LEARN (send blob values separately for Max4Live MIDI_LEARN)
  // LEDs : alternates durring all the learning process
  if (buttonB_ptr->rose() && buttonB_ptr->previousDuration() > LONG_HOLD) {
    *lastMode_ptr = *curentMode_ptr;
    *curentMode_ptr = MIDI_LEARN;
    encoder_ptr->write(0x1);
#if DEBUG_BUTTONS
    Serial.printf("\nMIDI_LEARN : %d", *curentMode_ptr);
#endif
  }
}

void update_presets(
  preset_t* presets_ptr,
  Encoder* encoder_ptr,
  uint8_t* curentMode_ptr,
  uint8_t* interpThreshold_ptr
) {
  switch (*curentMode_ptr) {
    case LINE_OUT:
      if (setLevel(&presets_ptr[LINE_OUT], encoder_ptr)) {
        presets_ptr[LINE_OUT].setLed = true;
        presets_ptr[LINE_OUT].update = true;
      }
      break;
    case SIG_IN:
      if (setLevel(&presets_ptr[SIG_IN], encoder_ptr)) {
        presets_ptr[SIG_IN].setLed = true;
        presets_ptr[SIG_IN].update = true;
      }
      break;
    case SIG_OUT:
      if (setLevel(&presets_ptr[SIG_OUT], encoder_ptr)) {
        presets_ptr[SIG_OUT].setLed = true;
        presets_ptr[SIG_OUT].update = true;
      }
      break;
    case THRESHOLD:
      if (setLevel(&presets_ptr[THRESHOLD], encoder_ptr)) {
        interpThreshold_ptr = constrain(presets_ptr[THRESHOLD].val - 5, presets_ptr[THRESHOLD].minVal, presets_ptr[THRESHOLD].maxVal);
        presets_ptr[THRESHOLD].setLed = true;
        presets_ptr[THRESHOLD].update = true;
      }
      break;
    case MIDI_LEARN:
      if (setLevel(&presets_ptr[MIDI_LEARN], encoder_ptr)) {
        presets_ptr[MIDI_LEARN].setLed = true;
        presets_ptr[MIDI_LEARN].update = true;
      }
      break;
    default:
      break;
  }
}

// Setup LEDs according to the mode and rotary encoder values
void update_leds(
  preset_t* presets_ptr,
  uint8_t* curentMode_ptr,
  uint8_t* lastMode_ptr
) {
  static uint32_t timer = 0;
  static uint8_t iter = 0;

  switch (*curentMode_ptr) {

    case LINE_OUT:
      if (presets_ptr[LINE_OUT].setLed == true) {
        presets_ptr[LINE_OUT].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWriteFast(LED_PIN_D1, presets_ptr[LINE_OUT].D1);
        digitalWriteFast(LED_PIN_D2, presets_ptr[LINE_OUT].D2);
      }
      else if (presets_ptr[LINE_OUT].val != presets_ptr[LINE_OUT].lastVal) {
        uint8_t ledVal = map(presets_ptr[LINE_OUT].val, presets_ptr[LINE_OUT].minVal, presets_ptr[LINE_OUT].maxVal, 0, 255);
        analogWrite(LED_PIN_D1, abs(ledVal - 255));
        analogWrite(LED_PIN_D2, ledVal);
      }
      break;

    case SIG_IN:
      if (presets_ptr[SIG_IN].setLed == true) {
        presets_ptr[SIG_IN].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWriteFast(LED_PIN_D1, presets_ptr[SIG_IN].D1);
        digitalWriteFast(LED_PIN_D2, presets_ptr[SIG_IN].D2);
      }
      else if (presets_ptr[SIG_IN].val != presets_ptr[SIG_IN].lastVal) {
        uint8_t ledVal = map(presets_ptr[SIG_IN].val, presets_ptr[SIG_IN].minVal, presets_ptr[SIG_IN].maxVal, 0, 255);
        analogWrite(LED_PIN_D1, abs(ledVal - 255));
        analogWrite(LED_PIN_D2, ledVal);
      }
      break;

    case SIG_OUT:
      if (presets_ptr[SIG_OUT].setLed == true) {
        presets_ptr[SIG_OUT].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWriteFast(LED_PIN_D1, presets_ptr[SIG_OUT].D1);
        digitalWriteFast(LED_PIN_D2, presets_ptr[SIG_OUT].D2);
      }
      else if (presets_ptr[SIG_OUT].val != presets_ptr[SIG_OUT].lastVal) {
        uint8_t ledVal = map(presets_ptr[SIG_OUT].val, presets_ptr[SIG_OUT].minVal, presets_ptr[SIG_OUT].maxVal, 0, 255);
        analogWrite(LED_PIN_D1, abs(ledVal - 255));
        analogWrite(LED_PIN_D2, ledVal);
      }
      break;

    case THRESHOLD:
      if (presets_ptr[THRESHOLD].setLed == true) {
        presets_ptr[THRESHOLD].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWriteFast(LED_PIN_D1, presets_ptr[THRESHOLD].D1);
        digitalWriteFast(LED_PIN_D2, presets_ptr[THRESHOLD].D2);
      }
      else if (presets_ptr[THRESHOLD].val != presets_ptr[THRESHOLD].lastVal) {
        uint8_t ledVal = map(presets_ptr[THRESHOLD].val, presets_ptr[THRESHOLD].minVal, presets_ptr[THRESHOLD].maxVal, 0, 255);
        analogWrite(LED_PIN_D1, abs(ledVal - 255));
        analogWrite(LED_PIN_D2, ledVal);
      }
      break;

    case MIDI_LEARN:
      // LEDs : alternating blink
      if (millis() - timer < MIDI_LEARN_LED_TIMEON && presets_ptr[MIDI_LEARN].setLed == true) {
        presets_ptr[MIDI_LEARN].setLed = false;
        digitalWriteFast(LED_PIN_D1, HIGH);
        digitalWriteFast(LED_PIN_D2, LOW);
      }
      else if (millis() - timer > MIDI_LEARN_LED_TIMEON && presets_ptr[MIDI_LEARN].setLed == false) {
        presets_ptr[MIDI_LEARN].setLed = true;
        digitalWriteFast(LED_PIN_D1, LOW);
        digitalWriteFast(LED_PIN_D2, HIGH);
      }
      else if (millis() - timer > MIDI_LEARN_LED_TIMEON + MIDI_LEARN_LED_TIMEOFF) {
        timer = millis();
      }
      break;

    case CALIBRATE:
      // LEDs : both LED are blinking three time
      if (iter < CALIBRATE_LED_ITER) {
        if (millis() - timer < CALIBRATE_LED_TIMEON && presets_ptr[CALIBRATE].setLed == true) {
          presets_ptr[CALIBRATE].setLed = false;
          digitalWriteFast(LED_PIN_D1, HIGH);
          digitalWriteFast(LED_PIN_D2, HIGH);
        }
        else if (millis() - timer > CALIBRATE_LED_TIMEON && presets_ptr[CALIBRATE].setLed == false) {
          presets_ptr[CALIBRATE].setLed = true;
          digitalWriteFast(LED_PIN_D1, LOW);
          digitalWriteFast(LED_PIN_D2, LOW);
        }
        else if (millis() - timer > CALIBRATE_LED_TIMEON + CALIBRATE_LED_TIMEOFF) {
          timer = millis;
          iter++;
          if (iter == CALIBRATE_LED_ITER) {
            *curentMode_ptr = *lastMode_ptr;
            *lastMode_ptr = LINE_OUT;
          }
        }
      }
      else {
        iter = 0;
      }
      break;

    case SAVE:
      // LEDs : Both LED are blinking weary fast
      if (iter < SAVE_LED_ITER) {
        if (millis() - timer  < SAVE_LED_TIMEON && presets_ptr[SAVE].setLed == true) {
          presets_ptr[SAVE].setLed = false;
          digitalWriteFast(LED_PIN_D1, HIGH);
          digitalWriteFast(LED_PIN_D2, HIGH);
        }
        else if (millis() - timer > SAVE_LED_TIMEON && presets_ptr[SAVE].setLed == false) {
          presets_ptr[SAVE].setLed = true;
          digitalWriteFast(LED_PIN_D1, LOW);
          digitalWriteFast(LED_PIN_D2, LOW);
        }
        else if (millis() - timer > SAVE_LED_TIMEON + SAVE_LED_TIMEOFF) {
          timer = millis();
          iter++;
          if (iter == SAVE_LED_ITER) {
            *curentMode_ptr = *lastMode_ptr;
            *lastMode_ptr = LINE_OUT;
          }
        }
      }
      else {
        iter = 0;
      }
      break;

    default:
      break;
  }
}

// Values adjustment using rotary encoder
boolean setLevel(preset_t* preset_ptr, Encoder* encoder_ptr) {

  preset_ptr->lastVal = preset_ptr->val;
  preset_ptr->val = encoder_ptr->read() >> 2;

  if (preset_ptr->val != preset_ptr->lastVal) {

    if (preset_ptr->val > preset_ptr->maxVal) {
      preset_ptr->val--;
      encoder_ptr->write(preset_ptr->val << 2);
      return false;
    }
    else if (preset_ptr->val < preset_ptr->minVal) {
      preset_ptr->val++;
      encoder_ptr->write(preset_ptr->val << 2);
      return false;
    }
    //preset_ptr->lastVal = preset_ptr->val;
    return true;
  }
  else {
    return false;
  }
}

// TODO
void preset_load(preset_t* preset_ptr, boolean * state_ptr) {

  for (int i = 0; i < 4; i++) {
    // EEPROM.read(i, preset_ptr[i].val); // uint8_t
  }
  *state_ptr = false;
}

// TODO
void preset_save(preset_t* preset_ptr, boolean * state_ptr) {

  for (int i = 0; i < 4; i++) {
    //EEPROM.write(i, preset_ptr[i].val); // uint8_t
  }
  *state_ptr = false;
}
