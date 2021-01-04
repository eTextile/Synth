/*
  This file is part of the eTextile-Synthetizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "hardware.h"

void SETUP_LEDS(void) {
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
}

void SETUP_SWITCHES(Button* buttonA_ptr, Button* buttonB_ptr) {
  buttonA_ptr->attach(BUTTON_PIN_L, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  buttonB_ptr->attach(BUTTON_PIN_R, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  buttonA_ptr->interval(15);                        // Debounce interval of 15 millis
  buttonB_ptr->interval(15);                        // Debounce interval of 15 millis
}

void e256_update_buttons(
  Button* buttonA_ptr,
  Button* buttonB_ptr,
  Encoder* encoder_ptr,
  preset_t* presets_ptr,
  uint8_t* curentMode_ptr,
  uint8_t* lastMode_ptr,
  uint8_t* iter_ptr,
  boolean* toggleSwitch_ptr,
  elapsedMillis* timer_ptr
) {

  buttonA_ptr->update();
  buttonB_ptr->update();

  // ACTION : BUTTON_L short press
  // FONCTION : CALIBRATE THE SENSOR MATRIX
  if (buttonA_ptr->rose() && buttonA_ptr->previousDuration() < LONG_HOLD) {
    *toggleSwitch_ptr = true;
    pinMode(LED_PIN_D1, OUTPUT);
    pinMode(LED_PIN_D2, OUTPUT);
    *iter_ptr = 0;
    *lastMode_ptr = *curentMode_ptr;
    *curentMode_ptr = CALIBRATE;
#if DEBUG_BUTTONS == 1
    Serial.printf("\nBUTTON_L : CALIBRATE : %d", *curentMode_ptr);
#endif
  }

  // ACTION : BUTTON_L long press
  // FONCTION : SAVE ALL PARAMETERS TO THE EEPROM MEMORY
  if (buttonA_ptr->rose() && buttonA_ptr->previousDuration() > LONG_HOLD) {
    *toggleSwitch_ptr = true;
    pinMode(LED_PIN_D1, OUTPUT);
    pinMode(LED_PIN_D2, OUTPUT);
    *iter_ptr = 0;
    *lastMode_ptr = *curentMode_ptr;
    *curentMode_ptr = SAVE;
#if DEBUG_BUTTONS == 1
    Serial.printf("\nBUTTON_L : SAVE : %d", *curentMode_ptr);
#endif
  }

  // ACTION : BUTTON_R short press
  // FONCTION : SELECT_MODE
  if (buttonB_ptr->rose() && buttonB_ptr->previousDuration() < LONG_HOLD) {
    *timer_ptr = 0;
    *lastMode_ptr = *curentMode_ptr;                                // Save last Mode
    *curentMode_ptr = (*curentMode_ptr + 1) % 4;                    // Loop into the modes
    encoder_ptr->write(presets_ptr[*curentMode_ptr].val << 2);      // Set encoder value
#if DEBUG_BUTTONS == 1
    Serial.printf("\nBUTTON_R : SELECT_MODE : %d", *curentMode_ptr);
#endif
  }

  // ACTION : BUTTON_R long press
  // FONCTION : MIDI_LEARN (send blob values separately for Max4Live MIDI_LEARN)
  // LEDs : alternates durring all the learning process
  if (buttonB_ptr->rose() && buttonB_ptr->previousDuration() > LONG_HOLD) {
    *timer_ptr = 0;
    encoder_ptr->write(0xFFFF);
    *lastMode_ptr = *curentMode_ptr;
    *curentMode_ptr = MIDI_LEARN;
#if DEBUG_BUTTONS == 1
    Serial.printf("\nMIDI_LEARN : %d", *curentMode_ptr);
#endif
  }
}

//
void e256_update_preset(
  Encoder* encoder_ptr,
  preset_t* preset_ptr,
  uint8_t* threshold_ptr,
  uint8_t* blobSelector_ptr,
  boolean* calibrate_ptr,
  boolean* save_ptr,
  AudioControlSGTL5000* dac_ptr,
  elapsedMillis* timer_ptr
) {

  switch (preset_ptr->mode) {
    // FONCTION : line_out level adjustment using rotary encoder // DEFAULT MODE
    // According to https://github.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
    // LOWEST level is 31 (1.16 Volts p-p)
    // HIGHEST level is 13 (3.16 Volts p-p)
    case LINE_OUT:
      if (setLevel(preset_ptr, encoder_ptr)) {
#if DEBUG_ENCODER == 1
        Serial.printf("\nLINE_OUT : %d", abs((preset_ptr->val - preset_ptr->minVal) - (preset_ptr->maxVal - preset_ptr->minVal)) + preset_ptr->minVal);
#else
        dac_ptr->dacVolume(abs((preset_ptr->val - preset_ptr->minVal) - (preset_ptr->maxVal - preset_ptr->minVal)) + preset_ptr->minVal);
#endif
      }
      break;

    // FONCTION : sig-in level adjustment using rotary encoder
    // According to https://githpresetub.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
    // LOWEST level is 15 (0.24 Volts p-p)
    // HIGHEST level is 0 (3.12 Volts p-p)
    case SIG_IN:
      if (setLevel(preset_ptr, encoder_ptr)) {
#if DEBUG_ENCODER == 1
        Serial.printf("\nSIG_IN : %d", abs((preset_ptr->val - preset_ptr->minVal) - (preset_ptr->maxVal - preset_ptr->minVal)) + preset_ptr->minVal);
#else
        dac_ptr->lineInLevel(abs((preset_ptr->val - preset_ptr->minVal) - (preset_ptr->maxVal - preset_ptr->minVal)) + preset_ptr->minVal);
#endif
      }
      break;

    // FONCTION : sig_out level adjustment using rotary encoder
    // According to https://github.com/PaulStoffregen/Audio/blob/master/control_sgtl5000.cpp
    // LOWEST level is 31
    // HIGHEST level is 13
    case SIG_OUT:
      if (setLevel(preset_ptr, encoder_ptr)) {
#if DEBUG_ENCODER == 1
        Serial.printf("\nSIG_OUT : %d", abs((preset_ptr->val - preset_ptr->minVal) - (preset_ptr->maxVal - preset_ptr->minVal)) + preset_ptr->minVal);
#else
        dac_ptr->lineOutLevel(abs((preset_ptr->val - preset_ptr->minVal) - (preset_ptr->maxVal - preset_ptr->minVal)) + preset_ptr->minVal);
#endif
      }
      break;

    // FONCTION : zThreshold value adjustment using rotary encoder
    case THRESHOLD:
      if (setLevel(preset_ptr, encoder_ptr)) {
        *threshold_ptr = preset_ptr->val;
#if DEBUG_ENCODER == 1
        Serial.printf("\nTHRESHOLD : %d", *threshold_ptr);
#endif
      }
      break;

    // FONCTION : select the Blob value to transmit via MIDI using rotary encoder
    case MIDI_LEARN:
      if (setLevel(preset_ptr, encoder_ptr)) {
        *blobSelector_ptr = preset_ptr->val;
#if DEBUG_ENCODER == 1
        Serial.printf("\nMIDI_LEARN : %d", *blobSelector_ptr);
#endif
      }
      break;

    // FONCTION : Calittoggleogglebrate the eTextile sensor matrix
    case CALIBRATE:
      if (calibrate_ptr == false) {
        *calibrate_ptr = true;
#if DEBUG_BUTTONS == 1
        Serial.println("DO_CALIBRATE");
#endif
        *timer_ptr = 0;
      }
      break;

    // FONCTION : SAVE PARAMETERS TO THE EEPROM MEMORY [LINE_OUT, SIG_IN, SIG_OUT, THRESHOLD]
    // TODO : write and read at different locations to increase EEPROM life (1080 bytes)
    case SAVE:
      if (save_ptr == false) {
        *save_ptr = true;
#ifdef DEBUG_BUTTONS
        Serial.println("DO_SAVE");
#endif
        *timer_ptr = 0;
      }
      break;

    default:
      break;
  }
}

// Setup LEDs according to the mode and rotary encoder values
void e256_update_leds(
  preset_t* preset_ptr,
  elapsedMillis* timer_ptr,
  uint8_t* curentMode_ptr,
  uint8_t* lastMode_ptr
) {

  static uint8_t iter = 0;

  switch (preset_ptr->mode) {

    case LINE_OUT:
    case SIG_IN:
    case SIG_OUT:
    case THRESHOLD:
      if (*curentMode_ptr != *lastMode_ptr) {
        *lastMode_ptr = *curentMode_ptr;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWriteFast(LED_PIN_D1, preset_ptr->D1);
        digitalWriteFast(LED_PIN_D2, preset_ptr->D2);
      }
      else if (preset_ptr->val != preset_ptr->lastVal) {
        uint8_t ledVal = map(preset_ptr->val, preset_ptr->minVal, preset_ptr->maxVal, 0, 255);
        analogWrite(LED_PIN_D1, abs(ledVal - 255));
        analogWrite(LED_PIN_D2, ledVal);
      }
      break;

    case MIDI_LEARN:
      // LEDs : alternating blink
      if (*timer_ptr < MIDI_LEARN_LED_TIMEON && preset_ptr->toggle == true) {
        preset_ptr->toggle = false;
        digitalWriteFast(LED_PIN_D1, HIGH);
        digitalWriteFast(LED_PIN_D2, LOW);
      }
      else if (*timer_ptr > MIDI_LEARN_LED_TIMEON && preset_ptr->toggle == false) {
        preset_ptr->toggle = true;
        digitalWriteFast(LED_PIN_D1, LOW);
        digitalWriteFast(LED_PIN_D2, HIGH);
      }
      else if (*timer_ptr > MIDI_LEARN_LED_TIMEON << 1) {
        *timer_ptr = 0;
      }
      break;

    case CALIBRATE:
      // LEDs : Both LED are blinking three time
      if (iter <= CALIBRATE_LED_ITER) {
        if (*timer_ptr < CALIBRATE_LED_TIMEON && preset_ptr->toggle == true) {
          preset_ptr->toggle = false;
          digitalWriteFast(LED_PIN_D1, HIGH);
          digitalWriteFast(LED_PIN_D2, HIGH);
        }
        else if (*timer_ptr > CALIBRATE_LED_TIMEON && preset_ptr->toggle == false) {
          preset_ptr->toggle = true;
          digitalWriteFast(LED_PIN_D1, LOW);
          digitalWriteFast(LED_PIN_D2, LOW);
        }
        else if (*timer_ptr > CALIBRATE_LED_TIMEON + CALIBRATE_LED_TIMEOFF) {
          *timer_ptr = 0;
          iter++;
        }
      }
      else {
        iter = 0;
        *curentMode_ptr = *lastMode_ptr;
        *lastMode_ptr = LINE_OUT;
      }
      break;

    case SAVE:
      // LEDs : Both LED are blinking weary fast
      if (iter <= SAVE_LED_ITER) {
        if (*timer_ptr < SAVE_LED_TIMEON && preset_ptr->toggle == true) {
          preset_ptr->toggle = false;
          digitalWriteFast(LED_PIN_D1, HIGH);
          digitalWriteFast(LED_PIN_D2, HIGH);
        }
        else if (*timer_ptr > SAVE_LED_TIMEON && preset_ptr->toggle == false) {
          preset_ptr->toggle = true;
          digitalWriteFast(LED_PIN_D1, LOW);
          digitalWriteFast(LED_PIN_D2, LOW);
        }
        else if (*timer_ptr > SAVE_LED_TIMEON + SAVE_LED_TIMEOFF) {
          *timer_ptr = 0;
          iter++;
          Serial.println(iter);
        }
      }
      else {
        iter = 0;
        *curentMode_ptr = *lastMode_ptr;
        *lastMode_ptr = LINE_OUT;
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
      preset_ptr->lastVal = preset_ptr->val;
      preset_ptr->val--;
      encoder_ptr->write(preset_ptr->val << 2);
      return false;
    }
    else if (preset_ptr->val < preset_ptr->minVal) {
      preset_ptr->lastVal = preset_ptr->val;
      preset_ptr->val++;
      encoder_ptr->write(preset_ptr->val << 2);
      return false;
    }
    return true;
  }
  else {
    return false;
  }
}

//TODO
void e256_preset_load(preset_t* preset_ptr, boolean * state_ptr) {

  for (int i = 0; i < 4; i++) {
    //EEPROM.read(i, preset_ptr[i].val); // uint8_t
  }
  *state_ptr = false;
}

//TODO
void e256_preset_save(preset_t* preset_ptr, boolean * state_ptr) {

  for (int i = 0; i < 4; i++) {
    //EEPROM.write(i, preset_ptr[i].val); // uint8_t
  }
  *state_ptr = false;
}
