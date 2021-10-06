/*
 This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
 Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
 This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
 */

import themidibus.*; // Import the library
import javax.sound.midi.MidiMessage; // This is important because in this case we will use the MidiMessage class
import javax.sound.midi.ShortMessage;

MidiBus usbMIDI; // The MidiBus

String POROJECT_NAME = "ETEXTILE-SYNTH";
String MIDI_PORT_NAME = "E256";
String VERSION = "1.0.7";
String MIDI_PORT_IN = "MIDI [hw:1,0,0]";
String MIDI_PORT_OUT = "MIDI [hw:1,0,0]";

int MIDI_IN_CHANNEL = 1; // [1:X] Set the I/O MIDI channel

class blob_t {
  int  id;
  int  bx;
  int  by;
  int  bz;
  int  bw;
  int  bh;
};

final int BX = 3; // [3] Blob X centroid position
final int BY = 4; // [4] Blob Y centroid position
final int BZ = 5; // [5] Blob depth
final int BW = 6; // [6] Blob width
final int BH = 7; // [7] Blob Height

// MIDI_CONTROL_CHANGE
byte THRESHOLD =        3;
byte CALIBRATE =        4;
byte MIDI_BLOBS_PLAY =  6;
byte MIDI_BLOBS_LEARN = 7;
byte MIDI_RAW =         8;
byte MIDI_INTERP =      9;
byte MIDI_MAPPING =     10;
byte MIDI_OFF =         11;

ArrayList<MidiMessage> messages = new ArrayList<MidiMessage>();
ArrayList<MidiMessage> messagesCopy = new ArrayList<MidiMessage>();
ArrayList<blob_t> blobs = new ArrayList<blob_t>();

void setup() {
  size(800, 800, P3D);
  frameRate(60);
  stroke(255);
  strokeWeight(2);
  MidiBus.list();
  usbMIDI = new MidiBus(this, MIDI_PORT_IN, MIDI_PORT_OUT);
  //println(usbMIDI.attachedInputs());
  //println(usbMIDI.attachedOutputs());
};

void draw() {

  update();

  background(0);
  pushMatrix();
  blob_t blob;
  for (int i = 0; i < blobs.size(); i++) {
    blob = blobs.get(i);
    translate((width/256) * blob.bx, (height/256) * blob.by); 
    rotateX(0.5);
    noFill();
    box(blob.bw * 10, blob.bh * 10, blob.bz * 5);
  };
  popMatrix();
  delay(5);
};

void update() {
  messagesCopy.addAll(messages);
  messages.clear();
  for (int msg = 0; msg < messagesCopy.size(); msg++) {
    ShortMessage midiMsg = (ShortMessage) messagesCopy.get(msg);

    switch (midiMsg.getStatus()) {
    case ShortMessage.NOTE_ON:      
      //println("blob_ID_ON = :" + midiMsg.getData1());
      blob_t newBlob = new blob_t();
      newBlob.id = midiMsg.getData1();
      blobs.add(newBlob);
      break;
    case ShortMessage.NOTE_OFF:
      //println("blob_ID_OFF = :" + midiMsg.getData1());
      for (int b = 0; b < blobs.size(); b++) {
        if (blobs.get(b).id == midiMsg.getData1()) {
          blobs.remove(b);
          break;
        };
      };
      break;
    case ShortMessage.CONTROL_CHANGE:
      //println("blob_CHANNEL = :" + midiMsg.getChannel());
      for (int i = 0; i < blobs.size(); i++) {
        blob_t blobToUpdate = blobs.get(i);
        //println();
        if (blobToUpdate.id == (int)midiMsg.getChannel()) {
          print("ID:" + midiMsg.getChannel() + "_");
          switch(midiMsg.getData1()) {
          case BX:
            print(" X:" + midiMsg.getData2() + "_");
            blobToUpdate.bx = midiMsg.getData2();
            break;
          case BY:
            print(" Y:" + midiMsg.getData2() + "_");
            blobToUpdate.by = midiMsg.getData2();
            break;
          case BZ:
            print(" Z:" + midiMsg.getData2() +  "_");
            blobToUpdate.bz = midiMsg.getData2();
            break;
          case BW: 
            print(" W:" + midiMsg.getData2() + "_");
            blobToUpdate.bw = midiMsg.getData2();
            break;
          case BH: 
            print("H = :" + midiMsg.getData2());
            blobToUpdate.bh = midiMsg.getData2();
            break;
          default:
            break;
          };
        };
      };
      break;
    default:
      break;
    };
  };
  messagesCopy.clear();
};

void midiMessage(MidiMessage message) {
  messages.add(message);
};

void delay(int time) {
  int current = millis();
  while (millis () < current+time) {
    Thread.yield();
  };
};
