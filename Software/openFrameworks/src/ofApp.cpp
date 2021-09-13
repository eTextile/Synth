/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "ofApp.h"

void ofApp::setup() {
  ofSetVerticalSync(true);
  ofSetWindowTitle("E256 - " MIDI_PORT_NAME "_" VERSION);
  ofSetLogLevel(OF_LOG_VERBOSE);
  //midiIn.listInPorts(); // via instance -> comment this line when done
  //midiOut.listOutPorts(); // via instance -> comment this line when done
  //midiIn.openPort("MIDI_PORT_NAME");
  //midiOut.openPort("MIDI_PORT_NAME");
  midiIn.openPort(1);
  midiOut.openPort(1);
  midiIn.ignoreTypes(false, false, true);
  midiIn.addListener(this);
  midiIn.setVerbose(false); // print received messages to the console

  //FreeSansBold.load("./Data/FreeSansBold.ttf", 13, true, true);

  setTresholdSlider.addListener(this, &ofApp::E256_setTreshold);
  setCalirationButton.addListener(this, &ofApp::E256_setCaliration);
  setMidiLearnToggle.addListener(this, &ofApp::E256_setMidiLearn);
  getRawToggle.addListener(this, &ofApp::E256_getRaw);
  getInterpToggle.addListener(this, &ofApp::E256_getInterp);
  getBlobsToggle.addListener(this, &ofApp::E256_getBlobs);

  gui.setup("E256 - Parameters");
  gui.add(setTresholdSlider.setup("Threshold", 10, 0, 127));
  gui.add(setCalirationButton.setup("Calibrate"));
  gui.add(getRawToggle.setup("getRawData", false));
  gui.add(getInterpToggle.setup("getInterpData", false));
  gui.add(setMidiLearnToggle.setup("Midi Learn", false));
  gui.add(getBlobsToggle.setup("getBlobs", true));
  mode = MIDI_BLOBS;
  lastMode = MIDI_BLOBS;

  ofBackground(0);

  // 16 * 16
  for (int y = 0; y < RAW_ROWS; y++) {
    for (int x = 0; x < RAW_COLS; x++) {
      rawDataMesh.addVertex(ofPoint(x, y, 0));             // make a new vertex
      rawDataMesh.addColor(ofFloatColor(255, 255, 255));   // set vertex color to white
    }
  }
  for (int y = 0; y < RAW_ROWS - 1; y++) {
    for (int x = 0; x < RAW_COLS - 1; x++) {
      int i1 = x + y * RAW_COLS;           // 0, 1, 2, 3, 4
      int i2 = (x + 1) + y * RAW_COLS;     // 1, 2, 3, 4,
      int i3 = x + (y + 1) * RAW_COLS;     // 18, 19,
      int i4 = (x + 1) + (y + 1) * RAW_COLS;
      rawDataMesh.addTriangle(i1, i2, i4);
      rawDataMesh.addTriangle(i1, i3, i4);
    }
  }

  // 64 * 64
  for (int y = 0; y < NEW_ROWS; y++) {
    for (int x = 0; x < NEW_COLS; x++) {
      interpDataMesh.addVertex(ofPoint(x, y, 0));             // make a new vertex
      interpDataMesh.addColor(ofFloatColor(255, 255, 255));   // set vertex color to white
    }
  }
  for (int y = 0; y < NEW_COLS - 1; y++) {
    for (int x = 0; x < NEW_COLS - 1; x++) {
      int i1 = x + y * NEW_ROWS;           // 0, 1, 2, 3, 4
      int i2 = (x + 1) + y * NEW_ROWS;     // 1, 2, 3, 4,
      int i3 = x + (y + 1) * NEW_ROWS;     // 18, 19,
      int i4 = (x + 1) + (y + 1) * NEW_ROWS;
      interpDataMesh.addTriangle(i1, i2, i4);
      interpDataMesh.addTriangle(i1, i3, i4);
    }
  }
}

/////////////////////// UPDATE ///////////////////////
void ofApp::update() {

  if (mode == MIDI_RAW) {
    if (midiMessages.size() > 0) {
      ofxMidiMessage &message = midiMessages[0];
      if (message.status == MIDI_SYSEX) {
        for (size_t i = 0; i < message.bytes.size(); i++) {
          ofPoint point = rawDataMesh.getVertex(i);             // Get the point coordinates
          point.z = message.bytes[i + 1] * 3;                       // Change the z-coordinates
          rawDataMesh.setVertex(i, point);                      // Set the new coordinates
          rawDataMesh.setColor(i, ofColor(point.z, 0, 255));    // Change vertex color
        }
      }
    }
  }

  if (mode == MIDI_INTERP) {
    if (midiMessages.size() > 0) {
      ofxMidiMessage &message = midiMessages[1];
      if (message.status == MIDI_SYSEX) {
        for (size_t i = 0; i < message.bytes.size(); i++) {
          ofPoint point = rawDataMesh.getVertex(i);             // Get the point coordinates
          point.z = message.bytes[i + 1] * 3;                       // Change the z-coordinates
          interpDataMesh.setVertex(i, point);                   // Set the new coordinates
          interpDataMesh.setColor(i, ofColor(point.z, 0, 255)); // Change vertex color
        }
      }
    }
  }

  if (mode == MIDI_BLOBS) {
    if (midiMessages.size() > 0) {
      for (size_t i = 0; i < midiMessages.size(); ++i) {
        ofxMidiMessage &message = midiMessages[i];
        if (message.status == MIDI_NOTE_ON) {
          blob_t blob;
          blob.id = message.channel - 1;
          blobs.push_back(blob);
        }
        else if (message.status == MIDI_NOTE_OFF) {
          for (size_t i = 0; i < blobs.size(); ++i) {
            if (blobs[i].id == message.channel - 1) {
              blobs.erase(blobs.begin() + i);
            }
          }
        }
        else if (message.status == MIDI_CONTROL_CHANGE) {
          if (message.control == BY) blobs[message.channel - 1].cy = message.value;
          else if (message.control == BX) blobs[message.channel - 1].cx = message.value;
          else if (message.control == BW) blobs[message.channel - 1].width = message.value;
          else if (message.control == BH) blobs[message.channel - 1].height = message.value;
          else if (message.control == BD) blobs[message.channel - 1].depth = message.value;
        }
      }
    }
  }
}

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

  if (mode == MIDI_RAW) {
    ofPushMatrix();
    ofSetLineWidth(1);    // set line width to 1
    ofRotateDeg(30, 1, 0, 0);
    ofTranslate(ofGetWindowWidth() / 3, ofGetWindowHeight() / 8);
    ofScale(SCALE_H, SCALE_V, 1);
    rawDataMesh.drawWireframe(); // draws lines
    ofPopMatrix();
  }

  if (mode == MIDI_INTERP) {
    ofPushMatrix();
    ofSetLineWidth(1);    // set line width to 1
    ofRotateDeg(30, 1, 0, 0);
    ofTranslate(ofGetWindowWidth() / 3, ofGetWindowHeight() / 8);
    ofScale(SCALE_H / 4, SCALE_V / 4, 1);
    interpDataMesh.drawWireframe(); // draws lines
    ofPopMatrix();
  }

  if (mode == MIDI_BLOBS) {
    ofPushMatrix();
    ofRotateDeg(30, 1, 0, 0);
    for (size_t i = 0; i < blobs.size(); ++i) {
      blob_t &blob = blobs[i];
      //ofSetColor(245, 58, 135); // Pink
      //FreeSansBold.drawString(std::to_string(blob.id), (float)((blob.cx / 127) * ofGetWindowWidth()), (float)((blob.cy / 127) * ofGetWindowHeight()));
      ofBoxPrimitive box;
      ofSetLineWidth(1);
      ofSetColor(255);
      box.setMode(OF_PRIMITIVE_TRIANGLES);
      box.setResolution(1);
      box.set(blob.width * BLOB_SCALE, blob.height * BLOB_SCALE, blob.depth);
      box.setPosition(ofMap(blob.cx, 0, 127, 0, ofGetWidth() * 0.7), ofMap(blob.cy, 0, 127, 0, ofGetWidth() * 0.7), blob.depth);
      box.drawWireframe();
    }
    ofPopMatrix();
  }

}

// E256 matrix sensor - SET THRESHOLD
void ofApp::E256_setTreshold(int & tresholdValue) {
  lastMode = mode;
  mode = THRESHOLD;
  midiOut.sendControlChange(1, THRESHOLD, (int8_t)tresholdValue);
  mode = lastMode;
}
// E256 matrix sensor - SET CALIBRATION
void ofApp::E256_setCaliration() {
  lastMode = mode;
  mode = CALIBRATE;
  midiOut.sendControlChange(1, CALIBRATE, 0);
  mode = lastMode;
}
// E256 matrix sensor - SET MIDI LEARN MODE
void ofApp::E256_setMidiLearn(bool & val) {
  if (val == true) {
    mode = MIDI_LEARN;
    midiOut.sendControlChange(1, MIDI_LEARN, 1);
  } else {
    mode = MIDI_OFF;
    midiOut.sendControlChange(1, MIDI_LEARN, 0);
  }
}
// E256 matrix sensor - MATRIX RAW DATA REQUEST MODE
// 16*16 matrix row data request
void ofApp::E256_getRaw(bool & val) {
  if (val == true) {
    mode = MIDI_RAW;
    midiOut.sendControlChange(1, MIDI_RAW, 1);
  } else {
    mode = MIDI_OFF;
    midiOut.sendControlChange(1, MIDI_RAW, 0);
  }
}
// E256 matrix sensor - INTERPOLATED DATA REQUEST MODE
void ofApp::E256_getInterp(bool & val) {
  if (val == true) {
    mode = MIDI_INTERP;
    midiOut.sendControlChange(1, MIDI_INTERP, 1);
  } else {
    mode = MIDI_OFF;
    midiOut.sendControlChange(1, MIDI_INTERP, 0);
  }
}
// E256 matrix sensor - BLOBS REQUEST MODE
void ofApp::E256_getBlobs(bool & val) {
  if (val == true) {
    mode = MIDI_BLOBS;
    midiOut.sendControlChange(1, MIDI_BLOBS, 1);
  } else {
    mode = MIDI_OFF;
    midiOut.sendControlChange(1, MIDI_BLOBS, 0);
  }
}

void ofApp::exit() {
  midiIn.closePort();
  midiIn.removeListener(this);
  midiOut.closePort();
  setTresholdSlider.removeListener(this, &ofApp::E256_setTreshold);
  setCalirationButton.removeListener(this, &ofApp::E256_setCaliration);
  getRawToggle.removeListener(this, &ofApp::E256_getRaw);
  getInterpToggle.removeListener(this, &ofApp::E256_getInterp);
  setMidiLearnToggle.removeListener(this, &ofApp::E256_setMidiLearn);
  getBlobsToggle.removeListener(this, &ofApp::E256_getBlobs);
}

/////////////////////// MidiIN ///////////////////////
void ofApp::newMidiMessage(ofxMidiMessage & msg) {
  //idiMutex.lock();
  //midiMessages.clear();
  midiMessages.push_back(msg);
  while (midiMessages.size() > maxMessages) {
    midiMessages.erase(midiMessages.begin());
  }
  //midiMutex.unlock();
}

// E256 matrix sensor - Toggle full screen mode
void ofApp::keyPressed(int key) {
  switch (key) {
    case 'f':
      ofToggleFullscreen();
      break;
    default:
      break;
  }
}
