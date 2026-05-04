/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// Tap Tempo — MIDI Clock generator.
//
// Each finger tap on the switch calls tap_tempo_hit():
//   - timestamps are stored in a circular buffer (up to TAP_MAX_TAPS)
//   - average interval over stored taps → BPM
//   - clock_interval_us = (avg_ms × 1000) / MIDI_PPQN
//
// tap_tempo_clock_tick() is called every frame from mapping_lib_update().
// It fires usbMIDI + hardware MIDI TimingClock (0xF8) at 24 PPQN.

#include "midi_tap_tempo.h"
#include "usb_midi_io.h"
#include "hardware_midi_io.h"

static uint32_t tap_times[TAP_MAX_TAPS]; // circular buffer of tap timestamps (ms)
static uint8_t  tap_head  = 0;           // index of next write slot
static uint8_t  tap_count = 0;           // number of valid entries (capped at TAP_MAX_TAPS)
static uint32_t clock_interval_us = (60000000UL / ((uint32_t)DEFAULT_BPM * MIDI_PPQN));
static uint32_t last_clock_us = 0;
static bool     clock_running = false;

void tap_tempo_hit(void) {
  uint32_t now_ms = millis();

  // Reset history if the tap gap is too long (user stopped tapping).
  if (tap_count > 0) {
    uint8_t prev = (tap_head + TAP_MAX_TAPS - 1) % TAP_MAX_TAPS;
    if (now_ms - tap_times[prev] > TAP_WINDOW_MS) {
      tap_count = 0;
      tap_head  = 0;
    }
  }

  tap_times[tap_head] = now_ms;
  tap_head = (tap_head + 1) % TAP_MAX_TAPS;
  if (tap_count < TAP_MAX_TAPS) tap_count++;

  if (tap_count >= 2) {
    // Average interval over all stored taps.
    uint8_t oldest_idx = (tap_head + TAP_MAX_TAPS - tap_count) % TAP_MAX_TAPS;
    uint8_t newest_idx = (tap_head + TAP_MAX_TAPS - 1) % TAP_MAX_TAPS;
    uint32_t avg_ms = (tap_times[newest_idx] - tap_times[oldest_idx]) / (tap_count - 1);
    clock_interval_us = (avg_ms * 1000UL) / MIDI_PPQN;
    clock_running = true;
  }
}

void tap_tempo_clock_tick(void) {
  if (!clock_running) return;
  uint32_t now_us = micros();
  if (now_us - last_clock_us >= clock_interval_us) {
    last_clock_us = now_us;
    usb_midi_send_clock(); //TODO: Not used when STANDALONE_MODE!
    hardware_midi_send_clock();
  }
}
