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
  midiIn.listInPorts(); // via instance -> comment this line when done
  midiIn.openPort("MIDI_PORT_NAME");
  midiOut.listOutPorts(); // via instance -> comment this line when done
  midiOut.openPort("MIDI_PORT_NAME");
  midiIn.ignoreTypes(false, false, false);
  midiIn.addListener(this);
  midiIn.setVerbose(true); // print received messages to the console

  FreeSansBold.load("./Data/FreeSansBold.ttf", 13, true, true);

  setCalirationButton.addListener(this, &ofApp::E256_setCaliration);
  setTresholdSlider.addListener(this, &ofApp::E256_setTreshold);
  getRawDataToggle.addListener(this, &ofApp::E256_rawDataRequestStart);
  getInterpDataToggle.addListener(this, &ofApp::E256_interpDataRequestStart);
  getBinDataToggle.addListener(this, &ofApp::E256_binDataRequestStart);
  getBlobsToggle.addListener(this, &ofApp::E256_blobsRequestStart);

  gui.setup("E256 - Parameters");
  gui.add(setCalirationButton.setup("Calibrate"));
  gui.add(setTresholdSlider.setup("Threshold", 10, 0, 60));
  gui.add(getRawDataToggle.setup("getRawData", false));
  gui.add(getInterpDataToggle.setup("getInterpData", true));
  gui.add(getBinDataToggle.setup("getBinData", false));
  gui.add(getBlobsToggle.setup("getBlobs", false));

  ofBackground(0);

  // 16 * 16
  for (int y=0; y<RAW_ROWS; y++) {
        for (int x=0; x<RAW_COLS; x++) {
            rawDataMesh.addVertex(ofPoint(x, y, 0));             // make a new vertex
            rawDataMesh.addColor(ofFloatColor(255, 255, 255));   // set vertex color to white
        }
    }
    for (int y=0; y<RAW_ROWS-1; y++) {
      for (int x=0; x<RAW_COLS-1; x++) {
        int i1 = x + y * RAW_COLS;           // 0, 1, 2, 3, 4
        int i2 = (x+1) + y * RAW_COLS;       // 1, 2, 3, 4,
        int i3 = x + (y+1) * RAW_COLS;       // 18, 19,
        int i4 = (x+1) + (y+1) * RAW_COLS;
        rawDataMesh.addTriangle(i1, i2, i4);
        rawDataMesh.addTriangle(i1, i3, i4);
    }
  }

  // 64 * 64
  for (int y=0; y<NEW_ROWS; y++) {
        for (int x=0; x<NEW_COLS; x++) {
            interpDataMesh.addVertex(ofPoint(x, y, 0));             // make a new vertex
            interpDataMesh.addColor(ofFloatColor(255, 255, 255));   // set vertex color to white
        }
    }
    for (int y=0; y<NEW_COLS-1; y++) {
      for (int x=0; x<NEW_COLS-1; x++) {
        int i1 = x + y * NEW_ROWS;           // 0, 1, 2, 3, 4
        int i2 = (x+1) + y * NEW_ROWS;       // 1, 2, 3, 4,
        int i3 = x + (y+1) * NEW_ROWS;       // 18, 19,
        int i4 = (x+1) + (y+1) * NEW_ROWS;
        interpDataMesh.addTriangle(i1, i2, i4);
        interpDataMesh.addTriangle(i1, i3, i4);
    }
  }
}

/*
/////////////////////// SERIAL EVENT ///////////////////////
void ofApp::onSerialBuffer(const ofxIO::SerialBufferEventArgs& args) {

  if (getRawData) {
    //midiMessage.value
    for (int i=0; i<RAW_FRAME; i++) {
      rawValues[i] = message[i];
    }
    // Update vertices with the E256 raw sensor values
    for (int index=0; index<RAW_FRAME; index++) {
        ofPoint p = rawDataMesh.getVertex(index);    // Get the point coordinates
        p.z = rawValues[index];                      // Change the z-coordinates
        rawDataMesh.setVertex(index, p);             // Set the new coordinates
        rawDataMesh.setColor(index, ofColor(rawValues[index], 0, 255));    // Change vertex color
    }
    E256_dataRequest = false;
  }

  if (getInterpData) {
  //if (getInterpDataToggle.getParameter() == true){

    std::copy(args.buffer().begin(), args.buffer().end(), inputFrameBuffer);
    //ofLogNotice("ofApp::onSerialBuffer") << "E256 - Serial message size : "<< message.OSCmessage.size();
    //ofLogNotice("ofApp::onSerialBuffer") << "E256 - Serial message : " << message.OSCmessage;
    for (int i=0; i<NEW_FRAME; i++){
      interpValues[i] = inputFrameBuffer[i + offset];
      //ofLogNotice("ofApp::onSerialBuffer") << "INDEX_" << i << " val_" << interpValues[i];
    }
    // Update vertices with the E256 interpolated sensor values
    for (int index=0; index<NEW_FRAME; index++) {
      ofPoint p = interpDataMesh.getVertex(index);    // Get the point coordinates
      p.z = interpValues[index];                      // Change the z-coordinates
      interpDataMesh.setVertex(index, p);             // Set the new coordinates
      interpDataMesh.setColor(index, ofColor(interpValues[index], 0, 255));    // Change vertex color
    }
    E256_dataRequest = false;
  }

  if (getBlobs) {
  //if (getBlobsToggle.getParameter() == true){
    //ofLogNotice("ofApp::onSerialBuffer") << "E256 - Serial message size : "<< message.OSCmessage.size();
    //ofLogNotice("ofApp::onSerialBuffer") << "E256 - Serial message : " << message.OSCmessage;

    message.OSCmessage = args.buffer().toString();

    int offset = 18;
    int stringOffset = 0;
    //ofLogNotice("ofApp::onSerialBuffer") << "E256 - Serial message size : "<< message.OSCmessage.size();
    //ofLogNotice("ofApp::onSerialBuffer") << "E256 - Serial message : " << message.OSCmessage;

    Poco::RegularExpression regex("/b(.){10}"); // GET X bytes after the "/b"
    Poco::RegularExpression::Match theMatch;

    
    for (size_t i=0; i<message.OSCmessage.size(); i++){
      ofLogNotice("ofApp::onSerialBuffer") << "INDEX_" << i << " val_" << message.OSCmessage[i];
    }
    

    // Expand SLIP-OSC serial message to OSC messages
    // https://en.cppreference.com/w/cpp/regex

    ofxOscBundle bundle;
    while (regex.match(message.OSCmessage, stringOffset, theMatch)){
      std::string msg = std::string(message.OSCmessage, theMatch.offset, theMatch.length);

      ofxOscMessage oscMessage;
      oscMessage.setAddress("/b");

      oscMessage.addIntArg(msg[offset]);          // UID
      oscMessage.addBoolArg(msg[offset + 1]);     // state
      oscMessage.addBoolArg(msg[offset + 2]);     // lastState

      ofLogNotice("ofApp::onSerialBuffer") << "GET__________________" << oscMessage.getArgAsInt(0);

      //uint32_t tempX = (msg[offset + 3] << 24) | (msg[offset + 4] << 16) | (msg[offset + 5] << 8) | (msg[offset + 6]);
      //ofLogNotice("ofApp::onSerialBuffer") << "X : " << (float)tempX;

      oscMessage.addFloatArg(msg[offset + 3]);    // Xcentroide
      oscMessage.addFloatArg(msg[offset + 4]);    // Ycentroid
      oscMessage.addIntArg(msg[offset + 5]);      // boxW
      oscMessage.addIntArg(msg[offset + 6]);      // boxH
      oscMessage.addIntArg(msg[offset + 7]);      // boxD

      blobs.push_back(oscMessage);
      bundle.addMessage(oscMessage);

      stringOffset = theMatch.offset + theMatch.length;
    }
    sender.sendBundle(bundle);
    E256_dataRequest = false;
  }

}
*/

/////////////////////// UPDATE ///////////////////////
void ofApp::update() {
}

//////////////////////// DRAW ////////////////////////
void ofApp::draw() {
  ofBackground(0);
  gui.draw();

  std::stringstream dashboard;
  dashboard << "MIDI_IN: " << midiIn.getInPortName(midiIn.getPort()) << std::endl;
  dashboard << "MIDI_OUT: " << midiOut.getOutPortName(midiOut.getPort()) << std::endl;
  dashboard << "FPS: " << (int)ofGetFrameRate() << std::endl;
  ofDrawBitmapString(dashboard.str(), ofVec2f(20, 200));

  //const int x = 0;       // X ofset
  //const int y = 0;       // Y ofset FIXME : dont afect the matrix graph
  const int SCALE_H = 50;  // Scale 14
  const int SCALE_V = 50;  // Scale 14
  //const int BLOB_SCALE = 5;

  if (getRawData) {
    ofPushMatrix();
    ofSetLineWidth(1);    // set line width to 1
    ofRotateDeg(30, 1, 0, 0);
    ofTranslate(ofGetWindowWidth()/3, ofGetWindowHeight()/8);
    ofScale(SCALE_H, SCALE_V, 1);
    rawDataMesh.drawWireframe(); // draws lines
    ofPopMatrix();
    if (E256_dataRequest == false){
      E256_rawDataRequest();
    }
  }

  if (getInterpData) {
    ofPushMatrix();
    ofSetLineWidth(1);    // set line width to 1
    ofRotateDeg(30, 1, 0, 0);
    ofTranslate(ofGetWindowWidth()/3, ofGetWindowHeight()/8);
    ofScale(SCALE_H/4, SCALE_V/4, 1);
    interpDataMesh.drawWireframe(); // draws lines
    ofPopMatrix();
    if (E256_dataRequest == false){
      E256_interpDataRequest();
    }
  }

  if (getBlobs) {
    ofPushMatrix();
    ofRotateDeg(30, 1, 0, 0);

/*
    for (size_t index = 0; index < blobs.size(); ++index){
      if(blobs[index].getAddress() == "/b"){
        uint8_t blobID    = blobs[index].getArgAsInt(0) & 0xFF;
        uint8_t state     = blobs[index].getArgAsInt(1) & 0xFF;
        uint8_t lastState = blobs[index].getArgAsInt(2) & 0xFF;
        float Xcentroid   = blobs[index].getArgAsFloat(3);
        float Ycentroid   = blobs[index].getArgAsFloat(4);
        uint8_t boxW      = blobs[index].getArgAsInt(5) & 0xFF;
        uint8_t boxH      = blobs[index].getArgAsInt(6) & 0xFF;
        uint8_t boxD      = blobs[index].getArgAsInt(7) & 0xFF;

        //ofLog(OF_LOG_VERBOSE,"E256_INPUT: UID:%d STATE:%d LAST_STATE:%d CX:%f CY:%f BW:%d BH:%d BD:%d",blobID, state, lastState, Xcentroid, Ycentroid, boxW, boxH, boxD);

        Xcentroid = (Xcentroid / NEW_COLS) * (ofGetWindowWidth());
        Ycentroid = (Ycentroid / NEW_ROWS) * (ofGetWindowHeight());
        boxW = boxW * BLOB_SCALE;
        boxH = boxH * BLOB_SCALE;

        ofBoxPrimitive box;
        ofSetLineWidth(2);    // set line width to 1
        //ofTranslate(ofGetWindowWidth()/4, ofGetWindowHeight()/8);
        //ofScale(.6, .6, .6);

        if (state == 1){
          ofSetColor(245, 58, 135); // Pink
          FreeSansBold.drawString(std::to_string(blobID), (int)Xcentroid+boxW, (int)Ycentroid);

          ofSetColor(255);
          box.setMode(OF_PRIMITIVE_TRIANGLES);
          box.setResolution(1);
          box.set((float)boxW, (float)boxH, (float)boxD);
          box.setPosition((float)Xcentroid, (float)Ycentroid, (float)boxD/2);
          box.drawWireframe();
          // box.draw();
          }
        else {
          blobs.erase(blobs.begin() + index);
        }
      }
    }
    blobs.clear();
    ofPopMatrix();

    if (E256_dataRequest == false){
      E256_blobsRequest();
    }
*/
  }
}

// E256 matrix sensor - SET CALIBRATION
void ofApp::E256_setCaliration() {
  midiMutex.lock();
  midiOut.sendControlChange(1, 5, 0);
  midiMutex.unlock();
  //ofLogNotice("ofApp::E256_setCaliration") << "E256 - Calibrate";
}
// E256 matrix sensor - SET THRESHOLD
void ofApp::E256_setTreshold(int & tresholdValue) {
  midiMutex.lock();
  midiOut.sendControlChange(1, 3, (uint8_t)tresholdValue);
  midiMutex.unlock();
  //ofLogNotice("ofApp::E256_setTreshold") << "E256 - Threshold seted : " << (int8_t)tresholdValue;
}
// E256 matrix sensor - MATRIX RAW DATA REQUEST START
// 16*16 matrix row data request
void ofApp::E256_rawDataRequestStart(bool & val) {
  if (val == true) E256_rawDataRequest();
  getRawData = val;
}
// E256 matrix sensor - INTERPOLATED DATA REQUEST START
void ofApp::E256_interpDataRequestStart(bool & val) {
  if (val == true) E256_interpDataRequest();
  getInterpData = val;
}
// E256 matrix sensor - BIN DATA REQUEST START
void ofApp::E256_binDataRequestStart(bool & val) {
  if (val == true) E256_binDataRequest();
  getBinData = val;
}
// E256 matrix sensor - BLOBS REQUEST START
void ofApp::E256_blobsRequestStart(bool & val) {
  if (val == true) E256_binDataRequest();
  getBlobs = val;
}
// E256 matrix sensor - MATRIX DATA REQUEST
// 16*16 matrix row data request
void ofApp::E256_rawDataRequest() {
  E256_dataRequest = true;
}
// E256 matrix sensor - MATRIX DATA REQUEST
// 64*64 matrix interpolated data request
void ofApp::E256_interpDataRequest() {
  E256_dataRequest = true;
}
// E256 matrix sensor - MATRIX DATA REQUEST
// 64*64 matrix binary data request
void ofApp::E256_binDataRequest() {
  E256_dataRequest = true;
}

// E256 matrix sensor - BLOBS REQUEST
void ofApp::E256_blobsRequest() {
  E256_dataRequest = true;
}

void ofApp::exit() {
  
  midiIn.closePort();
  midiIn.removeListener(this);
  //midiOut.closePort();
  
  setCalirationButton.removeListener(this, &ofApp::E256_setCaliration);
  setTresholdSlider.removeListener(this, &ofApp::E256_setTreshold);
  getRawDataToggle.removeListener(this, &ofApp::E256_rawDataRequestStart);
  getInterpDataToggle.removeListener(this, &ofApp::E256_interpDataRequestStart);
  getBinDataToggle.removeListener(this, &ofApp::E256_binDataRequestStart);
  getBlobsToggle.removeListener(this, &ofApp::E256_blobsRequestStart);
}

/////////////////////// MidiIN ///////////////////////
void ofApp::newMidiMessage(ofxMidiMessage& msg) {
    midiMutex.lock();
	// add the latest message to the message queue
	midiMessages.push_back(msg);
	// remove any old messages if we have too many
	while(midiMessages.size() > maxMessages) {
		midiMessages.erase(midiMessages.begin());
	}
    midiMutex.unlock();
}

// E256 matrix sensor - Toggle full screen mode
void ofApp::keyPressed(int key) {
  switch(key) {
    case 'f':
    ofToggleFullscreen();
    break;
    default:
    break;
  }
}

