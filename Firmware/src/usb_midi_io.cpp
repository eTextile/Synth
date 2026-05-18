/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "usb_midi_io.h"

#include "midi_bus.h"
#include "config.h"
#include "scan.h"
#include "interp.h"
#include "llist.h"
#include "blob.h"
#include "allocate.h"

uint32_t boot_time = 0;
size_t sysEx_data_length = 0;
uint8_t* sysEx_data_ptr = NULL;

static void usb_midi_read_note_on(uint8_t, uint8_t, uint8_t);
static void usb_midi_read_note_off(uint8_t, uint8_t, uint8_t);
static void usb_read_control_change(uint8_t, uint8_t, uint8_t);
static void usb_read_after_touch_poly(uint8_t, uint8_t, uint8_t);
static void usb_read_pitch_bend(uint8_t, int);
static void usb_read_system_exclusive(const uint8_t*, uint16_t, bool);

// Register all USB MIDI message handlers and start the USB MIDI stack.
void usb_midi_setup(void) {
  usbMIDI.begin();
  usbMIDI.setHandleNoteOn(usb_midi_read_note_on);
  usbMIDI.setHandleNoteOff(usb_midi_read_note_off);
  usbMIDI.setHandleControlChange(usb_read_control_change);
  usbMIDI.setHandleAfterTouchPoly(usb_read_after_touch_poly);
  usbMIDI.setHandlePitchChange(usb_read_pitch_bend);
  usbMIDI.setHandleSystemExclusive(usb_read_system_exclusive);
};

// Poll the USB MIDI input — must be called every main-loop iteration so
// registered handlers fire promptly.
void usb_midi_receive(void) {
  usbMIDI.read();
};

// Send the raw (pre-interpolation) sensor frame as a single SysEx message.
// Throttled to MATRIX_MIDI_THROTTLE_MS to avoid flooding the USB bus.
void usb_midi_transmit_raw_matrix(void) {
  static uint32_t usb_transmit_time_stamp = 0;
  if (millis() - usb_transmit_time_stamp > MATRIX_MIDI_THROTTLE_MS) {
    usb_transmit_time_stamp = millis();
    usbMIDI.sendSysEx(RAW_FRAME, raw_frame.data_ptr, false);
    usbMIDI.send_now();
  }
  usb_midi_receive(); // process any incoming messages (e.g. PC to change mode)
};

// The interpolated frame (NEW_FRAME = 4096 bytes) exceeds the Teensy USB SysEx
// buffer (USB_MIDI_SYSEX_MAX = 290), so it is split into fixed-size chunks.
// Each chunk is prefixed with its 0-based index so the host can reassemble them.
// Throttled to MATRIX_MIDI_THROTTLE_MS to avoid flooding the USB bus.
static const uint16_t INTERP_CHUNK_SIZE = 256;
static const uint8_t  INTERP_NUM_CHUNKS = NEW_FRAME / INTERP_CHUNK_SIZE;

void usb_midi_transmit_interp_matrix(void) {
  static uint32_t usb_transmit_time_stamp = 0;
  if (millis() - usb_transmit_time_stamp > MATRIX_MIDI_THROTTLE_MS) {
    usb_transmit_time_stamp = millis();
    uint8_t chunk[INTERP_CHUNK_SIZE + 1];
    for (uint8_t i = 0; i < INTERP_NUM_CHUNKS; i++) {
      chunk[0] = i;
      memcpy(&chunk[1], interp_frame.data_ptr + (uint16_t)i * INTERP_CHUNK_SIZE, INTERP_CHUNK_SIZE);
      usbMIDI.sendSysEx(INTERP_CHUNK_SIZE + 1, chunk, false);
    }
    usbMIDI.send_now();
  }
  usb_midi_receive(); // process any incoming messages (e.g. PC to change mode)
};

// Send one B_COUNT-byte SysEx message per active blob (see blob_params_e in blob.h).
//
// Throttle strategy:
//   - PRESENT / MISSING blobs are sent at most once per MATRIX_MIDI_THROTTLE_MS
//     to limit USB traffic during steady-state tracking.
//   - NEW / FREE blobs bypass the throttle so the host always receives the
//     first-contact and last-contact events promptly. FREE blobs survive only
//     one extra matrix_find_blobs() cycle before being recycled, so any delay
//     would cause the host to miss the removal notification.
//
// send_now() is called once after the full loop to flush all blobs in a single
// USB transaction instead of one per blob.
void usb_midi_transmit_blobs(void) {
  static uint32_t usb_transmit_time_stamp = 0;

  uint32_t now = millis();
  bool send_steady = (now - usb_transmit_time_stamp) >= MATRIX_MIDI_THROTTLE_MS;

  uint8_t blob_msg[B_COUNT];
  bool any_sent = false;
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);

    bool is_event = (blob_ptr->status == NEW || blob_ptr->status == RELEASED || blob_ptr->status == FREE );
    if (!is_event && !send_steady) continue;

    blob_msg[B_STATUS]      = (uint8_t)blob_ptr->status;
    blob_msg[B_LAST_STATUS] = (uint8_t)blob_ptr->last_status;
    blob_msg[B_UID]         = blob_ptr->UID;
    uint8_t whole_part      = (uint8_t)blob_ptr->centroid.x;
    blob_msg[B_X_WHOLE]     = whole_part;
    blob_msg[B_X_FRAC]      = (uint8_t)((blob_ptr->centroid.x - whole_part) * 100);
    whole_part              = (uint8_t)blob_ptr->centroid.y;
    blob_msg[B_Y_WHOLE]     = whole_part;
    blob_msg[B_Y_FRAC]      = (uint8_t)((blob_ptr->centroid.y - whole_part) * 100);
    blob_msg[B_WIDTH]       = blob_ptr->box.w;
    blob_msg[B_HEIGHT]      = blob_ptr->box.h;
    blob_msg[B_DEPTH]       = blob_ptr->centroid.z;
    blob_msg[B_VELOCITY_XY] = (uint8_t)constrain((int)(blob_ptr->velocity.xy * 127.0f / VELOCITY_XY_MAX), 0, 127);
    blob_msg[B_VELOCITY_Z]  = (uint8_t)constrain(64 + (int)(blob_ptr->velocity.z * 64.0f / VELOCITY_Z_DISPLAY_MAX), 0, 127);
    blob_msg[B_ATTACK_Z]    = (uint8_t)constrain((int)(blob_ptr->velocity.attack_z * 127.0f / VELOCITY_ATTACK_Z_MAX), 0, 127);
    blob_msg[B_ATTACK_DONE] = blob_ptr->velocity.attack_done ? 1 : 0;
    usbMIDI.sendSysEx(B_COUNT, blob_msg, false);
    any_sent = true;
  }

  if (!any_sent) return;
  if (send_steady) usb_transmit_time_stamp = now;
  usbMIDI.send_now();
  usb_midi_receive();
};

// Forward all pending outbound MIDI messages from llist_midi_out to the USB host.
// Called in MAPPING / PLAY / THROUGH modes to deliver note-on/off, CC, etc.
void mapping_usb_midi_transmit(void) {
  for (lnode_t* midi_node_ptr = ITERATOR_START_FROM_HEAD(&llist_midi_out); midi_node_ptr != NULL; midi_node_ptr = ITERATOR_NEXT(midi_node_ptr)) {
    midi_msg_t* midi_msg_ptr = (midi_msg_t*)ITERATOR_DATA(midi_node_ptr);
    usbMIDI.send((uint8_t)midi_msg_ptr->type, midi_msg_ptr->data1, midi_msg_ptr->data2, midi_msg_ptr->channel, 0);
  }
  while (usbMIDI.read());
};

// Send a MIDI Real Time TimingClock (0xF8) to the USB host — called by tap_tempo_clock_tick().
void usb_midi_send_clock(void) {
  usbMIDI.sendClock();
};

void usb_midi_send_sysex_ack(uint8_t ack) {
  uint8_t pkt[3] = { SYSEX_DEVICE_ID, SYSEX_PKT_ACK, ack };
  usbMIDI.sendSysEx(3, pkt, false);
  usbMIDI.send_now();
  while (usbMIDI.read());
};

void usb_midi_send_sysex_err(uint8_t err) {
  uint8_t pkt[3] = { SYSEX_DEVICE_ID, SYSEX_PKT_ERR, err };
  usbMIDI.sendSysEx(3, pkt, false);
  usbMIDI.send_now();
  while (usbMIDI.read());
};

void usb_midi_send_sysex_param(uint8_t param_id, uint8_t value) {
  uint8_t pkt[4] = { SYSEX_DEVICE_ID, SYSEX_PKT_PARAM, param_id, value };
  usbMIDI.sendSysEx(4, pkt, false);
  usbMIDI.send_now();
  while (usbMIDI.read());
};

// Store an incoming Note On from the USB host.
// In THROUGH_MODE the message is forwarded directly to the hardware MIDI output.
static void usb_midi_read_note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (e256_current_mode != THROUGH_MODE) return;
  midi_msg_t* midi_msg_ptr = (midi_msg_t*)llist_pop_front(&llist_midi_nodes_pool);
  if (midi_msg_ptr != NULL) {
    midi_msg_ptr->channel = channel;
    midi_msg_ptr->type = NoteOn;
    midi_msg_ptr->data1 = note;
    midi_msg_ptr->data2 = velocity;
    llist_push_front(&llist_midi_out, midi_msg_ptr);
  }
};

// Store an incoming Note Off from the USB host.
// In THROUGH_MODE the message is forwarded directly to the hardware MIDI output.
static void usb_midi_read_note_off(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (e256_current_mode != THROUGH_MODE) return;
  midi_msg_t* midi_msg_ptr = (midi_msg_t*)llist_pop_front(&llist_midi_nodes_pool);
  if (midi_msg_ptr != NULL) {
    midi_msg_ptr->channel = channel;
    midi_msg_ptr->type = NoteOff;
    midi_msg_ptr->data1 = note;
    midi_msg_ptr->data2 = velocity;
    llist_push_front(&llist_midi_out, midi_msg_ptr);
  }
};

// Forward an incoming Control Change from the USB host to hardware MIDI in THROUGH_MODE.
static void usb_read_control_change(uint8_t channel, uint8_t control, uint8_t value) {
  if (e256_current_mode != THROUGH_MODE) return;
  midi_msg_t* midi_msg_ptr = (midi_msg_t*)llist_pop_front(&llist_midi_nodes_pool);
  if (midi_msg_ptr != NULL) {
    midi_msg_ptr->channel = channel;
    midi_msg_ptr->type = ControlChange;
    midi_msg_ptr->data1 = control;
    midi_msg_ptr->data2 = value;
    llist_push_front(&llist_midi_out, midi_msg_ptr);
  }
};

// Store an incoming Polyphonic Aftertouch from the USB host.
// In THROUGH_MODE the message is forwarded to the hardware MIDI output.
static void usb_read_after_touch_poly(uint8_t channel, uint8_t note, uint8_t pressure) {
  if (e256_current_mode != THROUGH_MODE) return;
  midi_msg_t* midi_msg_ptr = (midi_msg_t*)llist_pop_front(&llist_midi_nodes_pool);
  if (midi_msg_ptr != NULL) {
    midi_msg_ptr->channel = channel;
    midi_msg_ptr->type = AfterTouchPoly;
    midi_msg_ptr->data1 = note;
    midi_msg_ptr->data2 = pressure;
    llist_push_front(&llist_midi_out, midi_msg_ptr);
  }
};

// Store an incoming Pitch Bend from the USB host.
// The 14-bit value is split into LSB (data1) and MSB (data2).
// In THROUGH_MODE the message is forwarded to the hardware MIDI output.
static void usb_read_pitch_bend(uint8_t channel, int pitch) {
  if (e256_current_mode != THROUGH_MODE) return;
  midi_msg_t* midi_msg_ptr = (midi_msg_t*)llist_pop_front(&llist_midi_nodes_pool);
  if (midi_msg_ptr != NULL) {
    midi_msg_ptr->channel = channel;
    midi_msg_ptr->type = PitchBend;
    midi_msg_ptr->data1 = pitch & 0x7F; // Lsb
    midi_msg_ptr->data2 = pitch >> 7;   // Msb
    llist_push_front(&llist_midi_out, midi_msg_ptr);
  }
};

// Receive SysEx from the USB host.
//
// Three packet types are handled:
//   1. ALLOCATE: mode == ALLOCATE_MODE, host sends [ 0xF0, DEVICE_ID, SIZE_MSB, SIZE_LSB, 0xF7 ]
//                firmware allocates a buffer and replies ALLOCATE_DONE.
//   2. CMD:      [ 0xF0, DEVICE_ID, SYSEX_PKT_CMD, mode_value, 0xF7 ]
//                firmware switches mode and replies with ACK.
//   3. UPLOAD:   mode == UPLOAD_MODE, host sends JSON data in one or more chunks.
//                - Single chunk:  strip 2-byte header + 1-byte footer, reply UPLOAD_DONE.
//                - First chunk:   strip 2-byte header, advance pointer.
//                - Middle chunks: copy verbatim, advance pointer.
//                - Last chunk:    strip 1-byte footer, reply UPLOAD_DONE.
//
static void usb_read_system_exclusive(const uint8_t *data_ptr, uint16_t sysEx_chunk_size, bool complete) {
  static uint8_t *sysEx_chunk_ptr = NULL;
  static uint16_t sysEx_last_chunk_size = 0;
  static uint8_t sysEx_chunks = 0;
  static uint8_t sysEx_chunk_count = 0;

  // Phase 1: JSON allocation packet — must be checked before CMD detection
  if (e256_current_mode == ALLOCATE_MODE) {
    uint8_t sysEx_data_length_MSB = *(data_ptr + 2);
    uint8_t sysEx_data_length_LSB = *(data_ptr + 3);
    sysEx_data_length = sysEx_data_length_MSB << 7 | sysEx_data_length_LSB;

    sysEx_chunk_ptr = sysEx_data_ptr = (uint8_t *)allocate(sysEx_data_ptr, sysEx_data_length + 1);
    sysEx_chunks = (uint8_t)((sysEx_data_length + 3) / USB_MIDI_SYSEX_MAX); // +3 adding heder & footer size
    sysEx_last_chunk_size = (sysEx_data_length + 3) % USB_MIDI_SYSEX_MAX; // +3 adding heder & footer size

    if (sysEx_last_chunk_size != 0) sysEx_chunks++;

    usb_midi_send_sysex_ack((uint8_t)ALLOCATE_DONE);
    return;
  }

  // Phase 2: Control CMD packet [ 0xF0, DEVICE_ID, SYSEX_PKT_CMD, mode_value, 0xF7 ]
  if (sysEx_chunk_size >= 4 && data_ptr[1] == SYSEX_DEVICE_ID && data_ptr[2] == SYSEX_PKT_CMD) {
    switch (data_ptr[3]) {

      case PENDING_MODE:
        set_mode(PENDING_MODE);
        usb_midi_send_sysex_ack((uint8_t)PENDING_MODE_DONE);
        break;

      case SYNC_MODE:
        set_mode(SYNC_MODE);
        usb_midi_send_sysex_ack((uint8_t)SYNC_MODE_DONE);
        break;

      case MATRIX_RAW_MODE:
        set_mode(MATRIX_RAW_MODE);
        usb_midi_send_sysex_ack((uint8_t)MATRIX_RAW_MODE_DONE);
        break;

      case MATRIX_INTERP_MODE:
        set_mode(MATRIX_INTERP_MODE);
        usb_midi_send_sysex_ack((uint8_t)MATRIX_INTERP_MODE_DONE);
        break;

      case MAPPING_MODE:
        set_mode(MAPPING_MODE);
        usb_midi_send_sysex_ack((uint8_t)MAPPING_MODE_DONE);
        break;

      case EDIT_MODE:
        set_mode(EDIT_MODE);
        usb_midi_send_sysex_ack((uint8_t)EDIT_MODE_DONE);
        break;

      case THROUGH_MODE:
        set_mode(THROUGH_MODE);
        usb_midi_send_sysex_ack((uint8_t)THROUGH_MODE_DONE);
        break;

      case PLAY_MODE:
        set_mode(PLAY_MODE);
        usb_midi_send_sysex_ack((uint8_t)PLAY_MODE_DONE);
        break;

      case LOAD_MODE:
        if (load_flash_config()) {
          usb_midi_send_sysex_ack((uint8_t)LOAD_MODE_DONE);
        }
        else {
          usb_midi_send_sysex_err((uint8_t)NO_CONFIG_FILE);
        }
        break;

      case FETCH_MODE:
        usbMIDI.sendSysEx(flash_config_size, flash_config_ptr, false);
        usb_midi_send_sysex_ack((uint8_t)FETCH_MODE_DONE);
        break;

      case ALLOCATE_MODE:
        e256_current_mode = ALLOCATE_MODE;
        usb_midi_send_sysex_ack((uint8_t)ALLOCATE_MODE_DONE);
        break;

      case UPLOAD_MODE:
        e256_current_mode = UPLOAD_MODE;
        usb_midi_send_sysex_ack((uint8_t)UPLOAD_MODE_DONE);
        break;

      case APPLY_MODE:
        if (mappings_apply_config(sysEx_data_ptr, sysEx_data_length)) {
          usb_midi_send_sysex_ack((uint8_t)CONFIG_APPLY_DONE);
        }
        else {
          usb_midi_send_sysex_err((uint8_t)CONFIG_APPLY_FAILED);
          set_mode(ERROR_MODE);
        }
        break;

      case CALIBRATE_MODE:
        matrix_calibrate();
        blink(10, 50);
        usb_midi_send_sysex_ack((uint8_t)CALIBRATE_MODE_DONE);
        break;

      default:
        #if defined(USB_MIDI_SERIAL) && defined(DEBUG_MODES)
          Serial.println("MODE_NOT_HANDLED!");
        #endif
        break;
    }
    return;
  }

  // Phase 3: JSON upload data
  if (e256_current_mode == UPLOAD_MODE) {
    if (complete && sysEx_chunk_count == 0) { // All data in one callback (Teensy 4.x behaviour)
      memcpy(sysEx_chunk_ptr, data_ptr + 2, sysEx_data_length); // exact length, skips F0 + DEVICE_ID
      usb_midi_send_sysex_ack((uint8_t)UPLOAD_DONE);
    }
    else if (sysEx_chunks > 1 && sysEx_chunk_count == 0) { // First chunk
      memcpy(sysEx_chunk_ptr, data_ptr + 2, sizeof(uint8_t) * sysEx_chunk_size - 2); // -2 removing header size
      sysEx_chunk_ptr += (sysEx_chunk_size - 2);
      sysEx_chunk_count++;
    }
    else if (!complete) { // Middle chunks
      memcpy(sysEx_chunk_ptr, data_ptr, sizeof(uint8_t) * sysEx_chunk_size);
      sysEx_chunk_ptr += sysEx_chunk_size;
      sysEx_chunk_count++;
    }
    else { // Last chunk
      memcpy(sysEx_chunk_ptr, data_ptr, sizeof(uint8_t) * sysEx_chunk_size - 1); // -1 removing footer size
      sysEx_chunk_count = 0;
      usb_midi_send_sysex_ack((uint8_t)UPLOAD_DONE);
    }
    return;
  }

  usb_midi_send_sysex_err((uint8_t)UNKNOWN_SYSEX);
  set_mode(ERROR_MODE);
};
