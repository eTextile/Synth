/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "ofApp.h"

void ofApp::setup() {
  ofSetVerticalSync(true);
  ofSetWindowTitle(POROJECT_NAME);
  ofSetLogLevel(OF_LOG_VERBOSE);
  FreeSansBold.load("FreeSansBold.ttf", 14);
  //midiIn.listInPorts(); // via instance -> comment this line when done
  //midiOut.listOutPorts(); // via instance -> comment this line when done
  midiIn.openPort(1); // MIDI_CHANNEL_OMNI !?
  midiOut.openPort(1);
  //midiIn.openPort(MIDI_PORT_NAME);
  //midiOut.openPort(MIDI_PORT_NAME);
  midiIn.ignoreTypes(false, true, true);
  midiIn.addListener(this);
  midiIn.setVerbose(false); // print received messages to the console

  //FreeSansBold.load("./Data/FreeSansBold.ttf", 13, true, true);

  setTresholdSlider.addListener(this, &ofApp::E256_setTreshold);
  setCalirationButton.addListener(this, &ofApp::E256_setCaliration);
  getBlobsToggle.addListener(this, &ofApp::E256_getBlobs);
  getRawToggle.addListener(this, &ofApp::E256_getRaw);

  gui.setup("E256 - Parameters");
  gui.add(setTresholdSlider.setup("Threshold", 10, 0, 127));
  gui.add(setCalirationButton.setup("Calibrate"));
  gui.add(getRawToggle.setup("getRawData", false));
  gui.add(getBlobsToggle.setup("Midi Blobs", false));

  lastMode = MIDI_OFF;
  mode = MIDI_BLOBS_PLAY;
  midiInput.clear();
  midiInputCopy.clear();
  midiOut.sendControlChange(MIDI_OUTPUT_CHANNEL, MIDI_BLOBS_PLAY, 1);

  // 16 * 16
  for (int y = 0; y < RAW_ROWS; y++) {
    for (int x = 0; x < RAW_COLS; x++) {
      rawDataMesh.addVertex(ofPoint(x, y, 0));             // make a new vertex
      rawDataMesh.addColor(ofFloatColor(255, 255, 255));   // set vertex color to white
    };
  };
  for (int y = 0; y < RAW_ROWS - 1; y++) {
    for (int x = 0; x < RAW_COLS - 1; x++) {
      int i1 = x + y * RAW_COLS;           // 0, 1, 2, 3, 4
      int i2 = (x + 1) + y * RAW_COLS;     // 1, 2, 3, 4,
      int i3 = x + (y + 1) * RAW_COLS;     // 18, 19,
      int i4 = (x + 1) + (y + 1) * RAW_COLS;
      rawDataMesh.addTriangle(i1, i2, i4);
      rawDataMesh.addTriangle(i1, i3, i4);
    };
  };

  // 64 * 64
  for (int y = 0; y < NEW_ROWS; y++) {
    for (int x = 0; x < NEW_COLS; x++) {
      interpDataMesh.addVertex(ofPoint(x, y, 0));             // make a new vertex
      interpDataMesh.addColor(ofFloatColor(255, 255, 255));   // set vertex color to white
    };
  };
  for (int y = 0; y < NEW_COLS - 1; y++) {
    for (int x = 0; x < NEW_COLS - 1; x++) {
      int i1 = x + y * NEW_ROWS;           // 0, 1, 2, 3, 4
      int i2 = (x + 1) + y * NEW_ROWS;     // 1, 2, 3, 4,
      int i3 = x + (y + 1) * NEW_ROWS;     // 18, 19,
      int i4 = (x + 1) + (y + 1) * NEW_ROWS;
      interpDataMesh.addTriangle(i1, i2, i4);
      interpDataMesh.addTriangle(i1, i3, i4);
    };
  };
};

/////////////////////// UPDATE ///////////////////////
void ofApp::update() {

  midiMutex.lock();
  midiInputCopy.clear();
  copy(midiInput.begin(), midiInput.end(), back_inserter(midiInputCopy));
  midiInput.clear();
  midiMutex.unlock();

  for (size_t j = 0; j < midiInputCopy.size(); j++) {
    ofxMidiMessage &message = midiInputCopy[j];
    switch (mode) {
      case MIDI_RAW:
        if (message.status == MIDI_SYSEX) {
          for (int k = 0; k < 256; k++) {
            ofPoint point = rawDataMesh.getVertex(k);           // Get the point coordinates
            point.z = (float)message.bytes[k + 1];              // Change the z-coordinates
            rawDataMesh.setVertex(k, point);                    // Set the new coordinates
            rawDataMesh.setColor(k, ofColor(point.z, 0, 255));  // Change vertex color
          };
        };
        break;
      //case MIDI_BLOBS_PLAY || MIDI_BLOBS_LEARN:
      case MIDI_BLOBS_PLAY:
        if (message.status == MIDI_NOTE_ON) {
          ofLogNotice("ofApp::update") << "midiMessage NOTE_ON : " << message.pitch;
          blob_t blob;
          blob.id = message.pitch; // pitch is also call note
          blobs.push_back(blob);
        } else if (message.status == MIDI_NOTE_OFF) {
          ofLogNotice("ofApp::update") << "midiMessage NOTE_OFF : " << message.pitch;
          for (size_t m = 0; m < blobs.size(); m++) {
            if (blobs[m].id == message.pitch) {
              blobs.erase(blobs.begin() + m);
              break;
            };
          };
        } else if (message.status == MIDI_CONTROL_CHANGE) {
          //ofLogNotice("ofApp::update") << "E256 - midiMessage CONTROL_CHANGE_CHANNEL: " << channel;
          int channel = message.channel;
          switch (channel) {
            case BlobX:
              blobs[message.control - 1].bx = message.value;
              break;
            case BlobY:
              blobs[message.control - 1].by = message.value;
              break;
            case BlobZ:
              blobs[message.control - 1].bz = message.value;
              break;
            case BlobW:
              blobs[message.control - 1].bw = message.value;
              break;
            case BlobH:
              blobs[message.control - 1].bh = message.value;
              break;
            default:
              break;
          }; // end of switch();
        };
        break;
      default:
        break;
    }; // end of switch();
  };
};

//////////////////////// DRAW ////////////////////////
void ofApp::draw() {
  ofBackground(0);
  gui.draw();

  std::stringstream dashboard;
  dashboard << "MIDI_IN: " << midiIn.getInPortName(midiIn.getPort()) << std::endl;
  dashboard << "MIDI_OUT: " << midiOut.getPort() << ' ' << midiOut.getName() << std::endl;
  dashboard << "FPS: " << (int)ofGetFrameRate() << std::endl;
  ofDrawBitmapString(dashboard.str(), ofVec2f(20, 200));

  //const int x = 0;       // X ofset
  //const int y = 0;       // Y ofset FIXME : dont afect the matrix graph
  const int SCALE_H = 50;
  const int SCALE_V = 50;
  const int BLOB_SCALE = 10;

  switch (mode) {
    case MIDI_RAW:
      ofPushMatrix();
      ofSetLineWidth(1);
      ofRotateDeg(30, 1, 0, 0);
      ofTranslate(ofGetWindowWidth() / 4.2, ofGetWindowHeight() / 8);
      ofScale(SCALE_H, SCALE_V, 1);
      rawDataMesh.drawWireframe(); // draws lines
      ofPopMatrix();
      break;
    //case MIDI_BLOBS_PLAY || MIDI_BLOBS_LEARN:
    case MIDI_BLOBS_PLAY:
      ofPushMatrix();
      ofRotateDeg(30, 1, 0, 0);

      for (size_t i = 0; i < blobs.size(); ++i) {
        blob_t &blob = blobs[i];
        ofSetColor(245, 58, 135); // Pink
        FreeSansBold.drawString(std::to_string(blob.id),
        (float)(blob.bx * ((ofGetWindowWidth() - 20) / 127) - 100),
        (float)(blob.by * ((ofGetWindowHeight() - 20) / 127)));
        ofBoxPrimitive box;
        ofSetLineWidth(1);
        ofSetColor(255);
        box.setMode(OF_PRIMITIVE_TRIANGLES);
        box.setResolution(1);
        box.set(blob.bw * BLOB_SCALE, blob.bh * BLOB_SCALE, blob.bz);
        box.setPosition(
        (float)(blob.bx * ((ofGetWindowWidth() - 20) / 127)),
        (float)(blob.by * ((ofGetWindowHeight() - 20) / 127)), 0);
        box.drawWireframe();
        ofPopMatrix();
      };
      break;
    default:
      break;
  };
};

// E256 matrix sensor - SET THRESHOLD
void ofApp::E256_setTreshold(int & tresholdValue) {
  lastMode = mode;
  mode = THRESHOLD;
  midiOut.sendControlChange(MIDI_OUTPUT_CHANNEL, THRESHOLD, (int8_t)tresholdValue);
  mode = lastMode;
};

// E256 matrix sensor - SET CALIBRATION
void ofApp::E256_setCaliration() {
  lastMode = mode;
  mode = CALIBRATE;
  midiOut.sendControlChange(MIDI_OUTPUT_CHANNEL, CALIBRATE, 1);
  mode = lastMode;
};

// E256 matrix sensor - get MIDI_BLOBS
void ofApp::E256_getBlobs(bool & val) {
  if (val == true) {
    mode = MIDI_BLOBS_PLAY;
    midiOut.sendControlChange(MIDI_OUTPUT_CHANNEL, MIDI_BLOBS_PLAY, 1);
    midiInput.clear();
    midiInputCopy.clear();
  } else {
    mode = MIDI_BLOBS_LEARN;
    midiOut.sendControlChange(MIDI_OUTPUT_CHANNEL, MIDI_BLOBS_LEARN, 1);
    midiInput.clear();
    midiInputCopy.clear();
  };
};

// E256 matrix sensor - MATRIX RAW DATA REQUEST MODE
// 16*16 matrix row data request
void ofApp::E256_getRaw(bool & val) {
  if (val == true) {
    mode = MIDI_RAW;
    midiOut.sendControlChange(MIDI_OUTPUT_CHANNEL, MIDI_RAW, 1);
    midiOut.sendControlChange(MIDI_OUTPUT_CHANNEL, MIDI_RAW, 1);
    midiInput.clear();
    midiInputCopy.clear();
  } else {
    mode = MIDI_OFF;
    midiOut.sendControlChange(MIDI_OUTPUT_CHANNEL, MIDI_OFF, 1);
    midiInput.clear();
    midiInputCopy.clear();
  };
};

void ofApp::exit() {
  midiIn.closePort();
  midiIn.removeListener(this);
  midiOut.closePort();
  setTresholdSlider.removeListener(this, &ofApp::E256_setTreshold);
  setCalirationButton.removeListener(this, &ofApp::E256_setCaliration);
  getRawToggle.removeListener(this, &ofApp::E256_getRaw);
  getBlobsToggle.removeListener(this, &ofApp::E256_getBlobs);
};

/////////////////////// MidiIN ///////////////////////
void ofApp::newMidiMessage(ofxMidiMessage & msg) {
  midiMutex.lock();
  midiInput.push_back(msg);
  // remove any old messages if we have too many
  while(midiInput.size() >= 255) {
    midiInput.erase(midiInput.begin());
  };
  midiMutex.unlock();
};

// E256 matrix sensor - Toggle full screen mode
void ofApp::keyPressed(int key) {
  switch (key) {
    case 'f':
      ofToggleFullscreen();
      break;
    default:
      break;
  };
};
