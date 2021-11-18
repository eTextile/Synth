/*
 This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
 Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
 This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
 */

// Import librarys

final String POROJECT_NAME = "ETEXTILE-SYNTH";
final String MIDI_PORT_NAME = "E256";
final String VERSION = "1.0.8";
final String MIDI_PORT_IN = "MIDI [hw:1,0,0]";
final String MIDI_PORT_OUT = "MIDI [hw:1,0,0]";

class blob_t {
  int id;
  int bx;
  int by;
  int bz;
  int bw;
  int bh;
};

final int BlobX = 3; // [3] Blob X centroid position
final int BlobY = 4; // [4] Blob Y centroid position
final int BlobZ = 5; // [5] Blob depth
final int BlobW = 6; // [6] Blob width
final int BlobH = 7; // [7] Blob Height

// E256_CONTROLS
final byte THRESHOLD =          3;
final byte CALIBRATE =          4;
final byte MIDI_BLOBS_PLAY =    6;
final byte MIDI_BLOBS_LEARN =   7;
final byte MIDI_MAPPING =       8;
final byte MIDI_RAW =           9;
final byte MIDI_INTERP =        10;
final byte MIDI_OFF =           11;
final byte SERIAL_BLOBS_PLAY =  12;

byte mode = SERIAL_BLOBS_PLAY;

ArrayList<blob_t> blobs = new ArrayList<blob_t>();

void setup() {
  size(800, 800, P3D);
  frameRate(60);
  stroke(255);
  strokeWeight(2);
  // Setup Web Serial using serial.js
  const serial = new Serial();  // Wait for the serial port to open.
  // Subscribe to the events.
  serial.on(SerialEvents.CONNECTION_OPENED, onSerialConnectionOpened);
  serial.on(SerialEvents.CONNECTION_CLOSED, onSerialConnectionClosed);
  serial.on(SerialEvents.DATA_RECEIVED, onSerialDataReceived);
  serial.on(SerialEvents.ERROR_OCCURRED, onSerialErrorOccurred);

  // Prompts user for approval to connect to a serial device and opens the port to
  // approved device
  async connectAndOpen(portFilters = null, serialOptions = {baudRate:9600});

  // Automatically connects and opens the previously approved port
  // If there are more than one, it takes the top port in the approved port list
  async autoConnectAndOpenPreviouslyApprovedPort(serialOptions = { baudRate: 9600 });

// Called by Serial when an error occurs
function onSerialErrorOccurred(eventSender, error) {
  console.log("onSerialErrorOccurred", error);
}

// Called by Serial when a serial connection opens
function onSerialConnectionOpened(eventSender) {
  console.log("onSerialConnectionOpened");
}

// Called by Serial when a connection closes
function onSerialConnectionClosed(eventSender) {
  console.log("onSerialConnectionClosed");
}

// Called by Serial when new data is received
function onSerialDataReceived(eventSender, newData) {
  console.log("onSerialDataReceived", newData);

  midiInputCopy.addAll(midiInput);
  midiInput.clear();
  for (int msg = 0; msg < midiInputCopy.size(); msg++) {
    switch (mode) {
    case MIDI_BLOBS_PLAY:
      ShortMessage midiMsg = (ShortMessage) midiInputCopy.get(msg);
      int status = -1;
      try {
        status = midiMsg.getCommand();
      }
      catch (Exception e) {
        print(e);
      }
      if (status == ShortMessage.NOTE_ON) {
        //if (midiMsg.getData1() != lastMidiMsg_ON.getData1()) {
        //lastMidiMsg_ON = midiMsg;
        println("blob_ID_ON: " + midiMsg.getData1());
        blob_t newBlob = new blob_t();
        newBlob.id = midiMsg.getData1();
        blobs.add(newBlob);
        //};
      } else if (status == ShortMessage.NOTE_OFF) {
        //if (midiMsg.getData1() != lastMidiMsg_OFF.getData1()) {
        //lastMidiMsg_OFF = midiMsg;
        println("blob_ID_OFF: " + midiMsg.getData1());
        for (int b = 0; b < blobs.size(); b++) {
          if (blobs.get(b).id == midiMsg.getData1()) {
            blobs.remove(b);
            break;
            //};
          };
        };
      } else if (status == ShortMessage.CONTROL_CHANGE) {
        //println("blob_Channel:" + midiMsg.getChannel());
        for (int i = 0; i < blobs.size(); i++) {
          blob_t blobToUpdate = blobs.get(i);

          if (midiMsg.getChannel() + 1 == blobToUpdate.id) {
            switch (midiMsg.getData1()) {
            case BlobX:
              //print(" X: " + midiMsg.getData2() + "_");
              blobToUpdate.bx = midiMsg.getData2();
              break;
            case BlobY:
              //print(" Y: " + midiMsg.getData2() + "_");
              blobToUpdate.by = midiMsg.getData2();
              break;
            case BlobZ:
              //print(" Z: " + midiMsg.getData2() +  "_");
              blobToUpdate.bz = midiMsg.getData2();
              break;
            case BlobW:
              //print(" W: " + midiMsg.getData2() + "_");
              blobToUpdate.bw = midiMsg.getData2();
              break;
            case BlobH:
              //print(" H: " + midiMsg.getData2());
              blobToUpdate.bh = midiMsg.getData2();
              break;
            default:
              break;
            };
          };
        };
      };
    case MIDI_RAW:
      // TODO SYSEX
      break;
    default:
      break;
    };
  };
  midiInputCopy.clear();
};

void draw() {
  update();
  background(0);
  pushMatrix();
  for (int i = 0; i < blobs.size(); i++) {
    blob_t theBlob = blobs.get(i);
    textSize(50);
    text(theBlob.id, theBlob.bx * (height/127), theBlob.by * (height/127));
    translate(theBlob.bx * (height/127), theBlob.by * (height/127));
    //rotateZ(0.5);
    noFill();
    box(theBlob.bw * 10, theBlob.bh * 10, theBlob.bz * 5);
  };
  popMatrix();
};

void delay(int time) {
  int current = millis();
  while (millis () < current+time) {
    Thread.yield();
  };
};
