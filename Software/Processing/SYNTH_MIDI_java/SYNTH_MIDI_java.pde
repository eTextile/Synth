/*
 This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
 Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
 This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

// http://www.smallbutdigital.com/projects/themidibus/
import themidibus.*;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.SysexMessage;
import javax.sound.midi.InvalidMidiDataException;

MidiBus usbMIDI;

final String VERSION = "1.0.27";

// Input index 1 / Output index 2  (from MidiBus.list() output)
final int MIDI_IN_INDEX  = 1;
final int MIDI_OUT_INDEX = 2;

// Blob status codes — must match firmware blob.h status_code_e
final int FREE     = 0;
final int NEW      = 1;
final int PRESENT  = 2;
final int MISSING  = 3;
final int RELEASED = 4;

// SysEx blob message field indices — must match firmware blob.h blob_params_e
final int B_STATUS      = 0;
final int B_LAST_STATUS = 1;
final int B_UID         = 2;
final int B_X_WHOLE     = 3;
final int B_X_FRAC      = 4;
final int B_Y_WHOLE     = 5;
final int B_Y_FRAC      = 6;
final int B_WIDTH       = 7;
final int B_HEIGHT      = 8;
final int B_DEPTH       = 9;
final int B_VELOCITY_XY = 10;
final int B_VELOCITY_Z  = 11;
final int B_ATTACK_Z    = 12;
final int B_ATTACK_DONE = 13;
final int B_COUNT       = 14;

// SysEx control packet constants — must match firmware config.h
final int SYSEX_DEVICE_ID = 0x7D;
final int SYSEX_PKT_CMD   = 0x01;
final int SYSEX_PKT_ACK   = 0x02;
final int SYSEX_PKT_ERR   = 0x03;

// Mode codes — must match firmware config.h mode_code_e
final int EDIT_MODE = 6;

// Sensor grid dimensions (NEW_COLS x NEW_ROWS after interpolation)
final int GRID_COLS = 64;
final int GRID_ROWS = 64;

class Blob {
  int   uid;
  float x, y;
  int   z, w, h;
  int   vxy, vz, attackZ;
  boolean attackDone;
}

HashMap<Integer, Blob> blobs = new HashMap<Integer, Blob>();

void setup() {
  size(800, 800);
  frameRate(30);
  MidiBus.list();
  usbMIDI = new MidiBus(this, MIDI_IN_INDEX, MIDI_OUT_INDEX);
  println("Press [E] to enter EDIT mode");
}

void draw() {
  background(0);
  float scaleX = (float)width  / GRID_COLS;
  float scaleY = (float)height / GRID_ROWS;

  HashMap<Integer, Blob> snapshot;
  synchronized(blobs) {
    snapshot = new HashMap<Integer, Blob>(blobs);
  }

  for (Blob b : snapshot.values()) {
    float px = b.x * scaleX;
    float py = b.y * scaleY;
    float bw = b.w * scaleX;
    float bh = b.h * scaleY;
    float r  = b.z * 1.5;

    // Bounding box
    noFill();
    stroke(255, 200, 0);
    strokeWeight(1);
    rect(px - bw * 0.5, py - bh * 0.5, bw, bh);

    // Pressure circle
    fill(255, 150, 0, 180);
    noStroke();
    ellipse(px, py, r, r);

    // Labels
    fill(255);
    textSize(12);
    text("id:" + b.uid,       px + 6, py - 4);
    text("z:" + b.z,          px + 6, py + 8);
    text("vz:" + (b.vz - 64), px + 6, py + 20);
  }
}

void midiMessage(MidiMessage message) {
  if (!(message instanceof SysexMessage)) return;

  byte[] raw = message.getMessage();
  if (raw == null || raw.length < 2) return;

  if ((raw[1] & 0xFF) == SYSEX_DEVICE_ID) {
    handleControlPacket(raw);
    return;
  }

  if (raw.length < B_COUNT + 1) return;

  int status = raw[1 + B_STATUS] & 0xFF;
  int uid    = raw[1 + B_UID]    & 0xFF;

  if (status == FREE || status == RELEASED) {
    synchronized(blobs) { blobs.remove(uid); }
    return;
  }

  synchronized(blobs) {
    Blob b = blobs.get(uid);
    if (b == null) {
      b = new Blob();
      b.uid = uid;
      blobs.put(uid, b);
    }
    b.x          = (raw[1 + B_X_WHOLE] & 0xFF) + (raw[1 + B_X_FRAC] & 0xFF) / 100.0;
    b.y          = (raw[1 + B_Y_WHOLE] & 0xFF) + (raw[1 + B_Y_FRAC] & 0xFF) / 100.0;
    b.z          =  raw[1 + B_DEPTH]       & 0xFF;
    b.w          =  raw[1 + B_WIDTH]       & 0xFF;
    b.h          =  raw[1 + B_HEIGHT]      & 0xFF;
    b.vxy        =  raw[1 + B_VELOCITY_XY] & 0xFF;
    b.vz         =  raw[1 + B_VELOCITY_Z]  & 0xFF;
    b.attackZ    =  raw[1 + B_ATTACK_Z]    & 0xFF;
    b.attackDone = (raw[1 + B_ATTACK_DONE] & 0xFF) != 0;
  }
}

void handleControlPacket(byte[] raw) {
  if (raw.length < 4) return;
  int pktType = raw[2] & 0xFF;
  int value   = raw[3] & 0xFF;
  if      (pktType == SYSEX_PKT_ACK) println("ACK: " + value);
  else if (pktType == SYSEX_PKT_ERR) println("ERR: " + value);
}

void sendModeCommand(int mode) {
  if (usbMIDI == null) return;
  try {
    byte[] data = { (byte)SYSEX_DEVICE_ID, (byte)SYSEX_PKT_CMD, (byte)mode };
    SysexMessage sysex = new SysexMessage();
    sysex.setMessage(SysexMessage.SYSTEM_EXCLUSIVE, data, data.length);
    usbMIDI.sendMessage(sysex);
    println("Sent mode: " + mode);
  } catch (InvalidMidiDataException e) {
    println("sendModeCommand error: " + e.getMessage());
  }
}

void keyPressed() {
  if (key == 'e' || key == 'E') sendModeCommand(EDIT_MODE);
}
