/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "usb_midi_io.h"

#include "config.h"
#include "scan.h"
#include "interp.h"
#include "llist.h"
#include "blob.h"
#include "midi_bus.h"
#include "allocate.h"

#include "hardware_midi_io.h"

uint32_t bootTime = 0;
size_t sysEx_data_length = 0;
uint8_t* sysEx_data_ptr = NULL;

// Setup the USB_MIDI communication port
void usb_midi_setup(void) {
  usbMIDI.begin();
  usbMIDI.setHandleProgramChange(usb_read_program_change);
  usbMIDI.setHandleNoteOn(usb_read_note_on);
  usbMIDI.setHandleNoteOff(usb_read_note_off);
  usbMIDI.setHandleControlChange(usb_read_control_change);
  usbMIDI.setHandleAfterTouchPoly(usb_read_after_touch_poly);
  usbMIDI.setHandleSystemExclusive(usb_read_system_exclusive);
  usbMIDI.setHandleClock(usb_read_midi_clock);
};

void usb_midi_recive(void) {
  usbMIDI.read(); // Is there a MIDI incoming messages on any channel
};

void usb_midi_transmit_raw_matrix(void) {
  static uint32_t usbTransmitTimeStamp = 0;
  if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
    usbTransmitTimeStamp = millis();
    usbMIDI.sendSysEx(RAW_FRAME, raw_frame.data_ptr, false);
    usbMIDI.send_now();
  }
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
};


void usb_midi_transmit_interp_matrix(void) {
  static uint32_t usbTransmitTimeStamp = 0;
  // NOT_WORKING > see https://forum.pjrc.com/threads/28282-How-big-is-the-MIDI-receive-buffer
  // Interpolation will be made on the web_app side!
  if (millis() - usbTransmitTimeStamp > MIDI_TRANSMIT_INTERVAL) {
    usbTransmitTimeStamp = millis();
    usbMIDI.sendSysEx(NEW_FRAME, interp_frame.data_ptr, false, 0);
    usbMIDI.send_now();
  };
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
};

void usb_midi_transmit_blobs(void) {
  uint8_t blob_values[10] = {0};
  for (lnode_t* node_ptr = ITERATOR_START_FROM_HEAD(&llist_blobs); node_ptr != NULL; node_ptr = ITERATOR_NEXT(node_ptr)) {
    blob_t* blob_ptr = (blob_t*)ITERATOR_DATA(node_ptr);

    blob_values[0] = blob_ptr->UID;

    uint8_t whole_part = (uint8_t)blob_ptr->centroid.x;
    blob_values[1] = whole_part;
    blob_values[2] = (uint8_t)((blob_ptr->centroid.x - whole_part) * 100); // Fractional part

    whole_part = (uint8_t)blob_ptr->centroid.y;
    blob_values[3] = whole_part;
    blob_values[4] = (uint8_t)((blob_ptr->centroid.y - whole_part) * 100); // Fractional part

    blob_values[5] = blob_ptr->box.w;
    blob_values[6] = blob_ptr->box.h;
    blob_values[7] = blob_ptr->centroid.z;

    blob_values[8] = (uint8_t)blob_ptr->status;
    blob_values[9] = (uint8_t)blob_ptr->last_status;

    usbMIDI.sendSysEx(10, blob_values, false);
    usbMIDI.send_now();
    //Serial.printf("\nBLOB_STATUS: %s BLOB_LAST_STATUS: %s", get_blob_status_name(blob_ptr->status), get_blob_status_name(blob_ptr->last_status));
  };
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
};

void usb_midi_transmit_mappings_midi_msg(void) {
  // Send mappings MIDI messages to the host application
  for (lnode_t* midi_node_ptr = ITERATOR_START_FROM_HEAD(&midi_out); midi_node_ptr != NULL; midi_node_ptr = ITERATOR_NEXT(midi_node_ptr)) {
    midi_msg_t* midi_ptr = (midi_msg_t*)ITERATOR_DATA(midi_node_ptr);
    usbMIDI.send((uint8_t)midi_ptr->type, midi_ptr->data1, midi_ptr->data2, midi_ptr->channel, 0);
  }
  while (usbMIDI.read()); // Read and discard any incoming MIDI messages
};

void usb_midi_send_info(uint8_t msg, uint8_t channel) {
  usbMIDI.sendProgramChange(msg, channel); // ProgramChange(program, channel);
  usbMIDI.send_now();
  while (usbMIDI.read());
};

void usb_read_note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
  midi_msg_t* midi_ptr = (midi_msg_t*)llist_pop_front(&midi_nodes_pool);
  if (midi_ptr) {
    midi_ptr->type = NoteOn;
    midi_ptr->data1 = note;
    midi_ptr->data2 = velocity;
    midi_ptr->channel = channel;
    if (e256_current_mode == THROUGH_MODE) {
      llist_push_front(&midi_out, midi_ptr); // Add the node to the midi_out linked list
    }
  }
  else {
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LLIST)
      Serial.printf("\nNo more nodes left in the : midi_nodes_pool -> see usb_read_note_on()");
    #endif
    set_mode(ERROR_MODE);
  }
};

void usb_read_note_off(uint8_t channel, uint8_t note, uint8_t velocity) {
  midi_msg_t* midi_ptr = (midi_msg_t*)llist_pop_front(&midi_nodes_pool);
  if (midi_ptr) {
    midi_ptr->type = NoteOff;
    midi_ptr->data1 = note;
    midi_ptr->data2 = velocity;
    midi_ptr->channel = channel;
    if (e256_current_mode == THROUGH_MODE) {
      llist_push_front(&midi_out, midi_ptr); // Add the node to the midi_out linked list
    }
  }
  else {
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LLIST)
      Serial.printf("\nNo more nodes left in the : midi_nodes_pool -> see usb_read_note_off()");
    #endif
    set_mode(ERROR_MODE);
  }
};

// Used by USB_MIDI
// If the CC comes from usb it is forwarded to midi_out acting as MIDI thru
void usb_read_control_change(uint8_t channel, uint8_t control, uint8_t value) {
  midi_msg_t* midi_ptr = (midi_msg_t*)llist_pop_front(&midi_nodes_pool);
  if (midi_ptr) {
    midi_ptr->type = ControlChange;
    midi_ptr->data1 = control;
    midi_ptr->data2 = value;
    midi_ptr->channel = channel;
    if (e256_current_mode == THROUGH_MODE) {
      llist_push_front(&midi_out, midi_ptr); // Add the node to the midi_out linked list
    }
  }
  else {
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LLIST)
      Serial.printf("\nNo more nodes left in the : midi_nodes_pool -> see usb_read_control_change()");
    #endif
    set_mode(ERROR_MODE);
  }
};

// Used by USB_MIDI
// If the CC comes from usb it is forwarded to midi_out acting as MIDI thru
void usb_read_after_touch_poly(uint8_t channel, uint8_t note, uint8_t pressure) {
  midi_msg_t* midi_ptr = (midi_msg_t*)llist_pop_front(&midi_nodes_pool);
  if (midi_ptr) {
    midi_ptr->type = AfterTouchPoly;
    midi_ptr->data1 = note;
    midi_ptr->data2 = pressure;
    midi_ptr->channel = channel;
    if (e256_current_mode == THROUGH_MODE) {
      llist_push_front(&midi_out, midi_ptr); // Add the node to the midi_out linked list
    }
  }
  else {
    #if defined(USB_MIDI_SERIAL) && defined(DEBUG_LLIST)
      Serial.printf("\nNo more nodes left in the : midi_nodes_pool -> see usb_read_control_change()");
    #endif
    set_mode(ERROR_MODE);
  }
};

void usb_read_program_change(uint8_t channel, uint8_t program) {
  switch (channel) {
    case MIDI_MODES_CHANNEL:
      //Serial.printf("\nRECEIVED:\t%s", get_mode_name((enum_mode_codes_t)program));
      switch (program) {

        case PENDING_MODE:
          set_mode(PENDING_MODE);
          usb_midi_send_info((uint8_t)PENDING_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case SYNC_MODE:
          set_mode(SYNC_MODE);
          usb_midi_send_info((uint8_t)SYNC_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case MATRIX_MODE_RAW:
          set_mode(MATRIX_MODE_RAW);
          usb_midi_send_info((uint8_t)MATRIX_MODE_RAW_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case MAPPING_MODE:
          set_mode(MAPPING_MODE);
          usb_midi_send_info((uint8_t)MAPPING_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case EDIT_MODE:
          set_mode(EDIT_MODE);
          usb_midi_send_info((uint8_t)EDIT_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case THROUGH_MODE:
          set_mode(THROUGH_MODE);
          usb_midi_send_info((uint8_t)THROUGH_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case PLAY_MODE:
          set_mode(PLAY_MODE);
          usb_midi_send_info((uint8_t)PLAY_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case LOAD_MODE:
          if(load_flash_config()) {
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
              Serial.printf("\nSEND_FLASH_CONFIG: ");
              print_bytes(flash_config_ptr, flash_config_size);
            #endif
            usb_midi_send_info((uint8_t)LOAD_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          }
          else {
            usb_midi_send_info((uint8_t)NO_CONFIG_FILE, MIDI_ERROR_CHANNEL);
          }
          break;

        case FETCH_MODE:
          usbMIDI.sendSysEx(flash_config_size, flash_config_ptr, false);
          usb_midi_send_info((uint8_t)FETCH_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case ALLOCATE_MODE:
          e256_current_mode = ALLOCATE_MODE;
          usb_midi_send_info((uint8_t)ALLOCATE_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case UPLOAD_MODE:
          e256_current_mode = UPLOAD_MODE;
          usb_midi_send_info((uint8_t)UPLOAD_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          break;

        case APPLY_MODE:
          if (apply_config(sysEx_data_ptr, sysEx_data_length)) {
            usb_midi_send_info((uint8_t)APPLY_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
              Serial.printf("\nnCONFIG_APPLY_DONE");
            #endif
          }
          else {
            usb_midi_send_info((uint8_t)CONFIG_APPLY_FAILED, MIDI_ERROR_CHANNEL);
            #if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
              Serial.printf("\nCONFIG_APPLY_FAILED");
            #endif
            set_mode(ERROR_MODE);
          }
          break;

        case CALIBRATE_MODE:
          matrix_calibrate();
          blink(10);
          #if defined(USB_MIDI_SERIAL)
            usb_midi_send_info((uint8_t)CALIBRATE_MODE_DONE, MIDI_VERBOSITY_CHANNEL);
          #endif
          break;

        default:
          //Serial.println("OUT_OFF_RANGE!"); // DROP!
          break;
      }
      break;
  }
};

// TODO
void usb_read_midi_clock(void) {
#if defined(USB_MIDI_SERIAL) && defined(DEBUG_CONFIG)
  Serial.println("Clock");
#endif
};

// Load mapping config via MIDI system exclusive message
// As MIDI buffer is limited to (USB_MIDI_SYSEX_MAX) we must register the data in chunks!
// Recive: [ SYSEX_BEGIN, SYSEX_DEVICE_ID, SYSEX_SIZE_MSB, SYSEX_SIZE_LSB, SYSEX_END ] 
// Send: USBMIDI_CONFIG_ALLOC_DONE
// Recive: [ SYSEX_BEGIN, SYSEX_DEVICE_ID, SYSEX_DATA, SYSEX_END ]
// Send: USBMIDI_CONFIG_LOAD_DONE

// TODO: set_level((level_code_t)control, value); // TODO: move it to the system-exclusive communication line

void usb_read_system_exclusive(const uint8_t *data_ptr, uint16_t sysEx_chunk_size, bool complete) {
  static uint8_t *sysEx_chunk_ptr = NULL;
  static uint16_t sysEx_last_chunk_size = 0;
  static uint8_t sysEx_chunks = 0;
  static uint8_t sysEx_chunk_count = 0;

  switch (e256_current_mode) {

  case ALLOCATE_MODE:
    uint8_t sysEx_data_length_MSB;
    uint8_t sysEx_data_length_LSB;

    sysEx_data_length_MSB = *(data_ptr + 2);
    sysEx_data_length_LSB = *(data_ptr + 3);
    sysEx_data_length = sysEx_data_length_MSB << 7 | sysEx_data_length_LSB;

    sysEx_chunk_ptr = sysEx_data_ptr = (uint8_t *)allocate(sysEx_data_ptr, sysEx_data_length + 1);
    sysEx_chunks = (uint8_t)((sysEx_data_length + 3) / USB_MIDI_SYSEX_MAX); // +3 adding heder & footer size
    sysEx_last_chunk_size = (sysEx_data_length + 3) % USB_MIDI_SYSEX_MAX; // +3 adding heder & footer size
    
    if (sysEx_last_chunk_size != 0) sysEx_chunks++;

    usb_midi_send_info((uint8_t)ALLOCATE_DONE, MIDI_VERBOSITY_CHANNEL);
    break;

  case UPLOAD_MODE:
    if (sysEx_chunks == 1 && complete) { // Only one chunk to load
      memcpy(sysEx_chunk_ptr, data_ptr + 2, sizeof(uint8_t) * sysEx_chunk_size - 2); // -3 removing heder & footer size
      usb_midi_send_info((uint8_t)UPLOAD_DONE, MIDI_VERBOSITY_CHANNEL);
    }
    else if (sysEx_chunks > 1 && sysEx_chunk_count == 0) { // First chunk
      memcpy(sysEx_chunk_ptr, data_ptr + 2, sizeof(uint8_t) * sysEx_chunk_size - 2); // -2 removing heder size
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
      usb_midi_send_info((uint8_t)UPLOAD_DONE, MIDI_VERBOSITY_CHANNEL);
    };
    break;

  default:
    usb_midi_send_info((uint8_t)UNKNOWN_SYSEX, MIDI_ERROR_CHANNEL);
    set_mode(ERROR_MODE);
    break;
  };
};
