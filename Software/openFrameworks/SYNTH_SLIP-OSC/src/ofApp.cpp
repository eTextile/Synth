#include "ofApp.h"

void ofApp::setup(void) {
  ofSetVerticalSync(true);
  ofSetWindowTitle(NAME "_" PROJECT "_" VERSION);
  //ofSetLogLevel(OF_LOG_VERBOSE);
  FreeSansBold.load("FreeSansBold.ttf", 14);

  auto devicesInfo = SerialDeviceUtils::listDevices();
  ofLogNotice("ofApp::setup") << "Connected Devices: ";
  for (auto& SerialDevice: devicesInfo) ofLogNotice("ofApp::setup") << "\t" << SerialDevice;
  if (!devicesInfo.empty()) {
    bool success = serialDevice.setup(USB_PORT, BAUD_RATE);
    if (success) {
      serialDevice.registerAllEvents(this);
      ofLogNotice("ofApp::setup") << "Successfully setup: " << USB_PORT;
    } else {
      ofLogNotice("ofApp::setup") << "Unable to setup: " << USB_PORT;
    };
  } else {
    ofLogNotice("ofApp::setup") << "No devices connected!";
  };
  //sender.setup(HOST, UDP_OUTPUT_PORT); // OSC - UDP config
  //receiver.setup(UDP_INPUT_PORT); // SLIP-OSC via wifi

  setCalirationButton.addListener(this, &ofApp::E256_setCaliration);
  setTresholdSlider.addListener(this, &ofApp::E256_setTreshold);
  getRawDataToggle.addListener(this, &ofApp::E256_rawDataRequestStart);
  getInterpDataToggle.addListener(this, &ofApp::E256_interpDataRequestStart);
  getBlobsToggle.addListener(this, &ofApp::E256_blobsRequestStart);

  gui.setup("E256 - Parameters");
  gui.add(setCalirationButton.setup("Calibrate"));
  gui.add(setTresholdSlider.setup("Threshold", 20, 0, 100));
  gui.add(getRawDataToggle.setup("getRawData", true));
  gui.add(getInterpDataToggle.setup("getInterpData", false));
  gui.add(getBlobsToggle.setup("getBlobs", false));

  ofBackground(0);

  // 16 * 16
  for (int y=0; y<RAW_ROWS; y++) {
        for (int x=0; x<RAW_COLS; x++) {
            rawDataMesh.addVertex(ofPoint(x, y, 0));             // make a new vertex
            rawDataMesh.addColor(ofFloatColor(255, 255, 255));   // set vertex color to white
        };
    };
    for (int y=0; y<RAW_ROWS-1; y++) {
      for (int x=0; x<RAW_COLS-1; x++) {
        int i1 = x + y * RAW_COLS;           // 0, 1, 2, 3, 4
        int i2 = (x+1) + y * RAW_COLS;       // 1, 2, 3, 4,
        int i3 = x + (y+1) * RAW_COLS;       // 18, 19,
        int i4 = (x+1) + (y+1) * RAW_COLS;
        rawDataMesh.addTriangle(i1, i2, i4);
        rawDataMesh.addTriangle(i1, i3, i4);
    };
  };

  // 64 * 64
  for (int y=0; y<NEW_ROWS; y++) {
        for (int x=0; x<NEW_COLS; x++) {
            interpDataMesh.addVertex(ofPoint(x, y, 0));             // make a new vertex
            interpDataMesh.addColor(ofFloatColor(255, 255, 255));   // set vertex color to white
        };
    };
    for (int y=0; y<NEW_COLS-1; y++) {
      for (int x=0; x<NEW_COLS-1; x++) {
        int i1 = x + y * NEW_ROWS;           // 0, 1, 2, 3, 4
        int i2 = (x+1) + y * NEW_ROWS;       // 1, 2, 3, 4,
        int i3 = x + (y+1) * NEW_ROWS;       // 18, 19,
        int i4 = (x+1) + (y+1) * NEW_ROWS;
        interpDataMesh.addTriangle(i1, i2, i4);
        interpDataMesh.addTriangle(i1, i3, i4);
    };
  };
};

/////////////////////// SERIAL EVENT ///////////////////////
void ofApp::onOscMessage(const ofxOscMessage& message){
  OSCMessages.push_back(message);
  while (OSCMessages.size() > 255) {
    OSCMessages.erase(OSCMessages.begin());
  };
};

void ofApp::onSerialError(const ofxIO::SerialBufferErrorEventArgs& error){
  ofLogError() << "Got OSC Error: " << error.exception().displayText();
};

/////////////////////// UPDATE ///////////////////////
void ofApp::update(void) {

  for (size_t i = 0; i < OSCMessages.size(); i++) {
    ofxOscMessage OSCmsg = OSCMessages[i];
    switch (mode) {
      case RAW_MATRIX:
        if (OSCmsg.getAddress() == "/RAW") {
          ofLogNotice("ofApp::OSCSerialDevice") << "RAW_MATRIX_OSCmsg" << OSCmsg;
          for (int k = 0; k < RAW_FRAME; k++) {
            ofPoint point = rawDataMesh.getVertex(k);           // Get the point coordinates
            point.z = (float)OSCmsg.getArgAsInt(k);             // Change the z-coordinates
            rawDataMesh.setVertex(k, point);                    // Set the new coordinates
            rawDataMesh.setColor(k, ofColor(point.z, 0, 255));  // Change vertex color
          };
        };
        break;
      case INTERP_MATRIX:
        if (OSCmsg.getAddress() == "/INTERP") {
          ofLogNotice("ofApp::OSCSerialDevice") << "INTERP_MATRIX_OSCmsg" << OSCmsg;
          for (int k = 0; k < NEW_FRAME; k++) {
            ofPoint point = rawDataMesh.getVertex(k);           // Get the point coordinates
            point.z = (float)OSCmsg.getArgAsInt(k);             // Change the z-coordinates
            interpDataMesh.setVertex(k, point);                    // Set the new coordinates
            interpDataMesh.setColor(k, ofColor(point.z, 0, 255));  // Change vertex color
          };
        };
        break;
      case BLOBS_PLAY:
        if (OSCmsg.getAddress() == "/ON") {
          //ofLogNotice("ofApp::OSCSerialDevice") << "BLOBS_PLAY_OSCmsg_ON" << OSCmsg;
          blob_t blob;
          blob.id = OSCmsg.getArgAsInt(0); // pitch is also call note
          blobs.push_back(blob);
        };
        if (OSCmsg.getAddress() == "/UPDATE") {
          //ofLogNotice("ofApp::OSCSerialDevice") << "BLOBS_PLAY_OSCmsg_UPDATE" << OSCmsg;
          for (size_t m = 0; m < blobs.size(); m++) {
            if (blobs[m].id == OSCmsg.getArgAsInt(0)) {
              blobs[m].bx = OSCmsg.getArgAsInt(1);
              blobs[m].by = OSCmsg.getArgAsInt(2);
              blobs[m].bz = OSCmsg.getArgAsInt(3);
              blobs[m].bw = OSCmsg.getArgAsInt(4);
              blobs[m].bh = OSCmsg.getArgAsInt(5);
          };
        };
        if (OSCmsg.getAddress() == "/OFF") {
          //ofLogNotice("ofApp::OSCSerialDevice") << "BLOBS_PLAY_OSCmsg_OFF" << OSCmsg;
          for (size_t m = 0; m < blobs.size(); m++) {
            if (blobs[m].id == OSCmsg.getArgAsInt(0)) {
              blobs.erase(blobs.begin() + m);
              break;
            };
          };
        };
        break;
    };
  };
  OSCMessages.clear();
};

//////////////////////// DRAW ////////////////////////
void ofApp::draw(void) {

  ofBackground(0);
  gui.draw();
  std::stringstream dashboard;
  //dashboard << "     Connected to : " << serialDevice.port() << std::endl;
  //dashboard << "SLIP-OSC-OUT port : " << UDP_OUTPUT_PORT << std::endl;
  //dashboard << " SLIP-OSC-IN port : " << UDP_INPUT_PORT << std::endl;
  dashboard << "              FPS : " << (int)ofGetFrameRate() << std::endl;
  ofDrawBitmapString(dashboard.str(), ofVec2f(20, 200)); // Draw the GUI menu

  const int SCALE_H = 50;
  const int SCALE_V = 50;
  const int BLOB_SCALE = 5;

  switch(mode){
  case RAW_MATRIX:
    ofPushMatrix();
    ofSetLineWidth(1);
    ofRotateDeg(30, 1, 0, 0);
    ofTranslate(ofGetWindowWidth()/3, ofGetWindowHeight()/8);
    ofScale(SCALE_H, SCALE_V, 1);
    rawDataMesh.drawWireframe();
    ofPopMatrix();
  break;
  case INTERP_MATRIX:
    ofPushMatrix();
    ofSetLineWidth(1);
    ofRotateDeg(30, 1, 0, 0);
    ofTranslate(ofGetWindowWidth()/3, ofGetWindowHeight()/8);
    ofScale(SCALE_H/4, SCALE_V/4, 1);
    interpDataMesh.drawWireframe();
    ofPopMatrix();
  break;
  case BLOBS_PLAY:
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
    break;
    default:
    break;
  };
};

// E256 matrix sensor - MATRIX RAW DATA REQUEST START
// 16*16 matrix row data request
void ofApp::E256_rawDataRequestStart(bool & val) {
  mode = RAW_MATRIX;
};
// E256 matrix sensor - INTERPOLATED DATA REQUEST START
void ofApp::E256_interpDataRequestStart(bool & val) {
  mode = INTERP_MATRIX;
};
// E256 matrix sensor - BLOBS REQUEST START
void ofApp::E256_blobsRequestStart(bool & val) {
  mode = BLOBS_PLAY;
};

// E256 matrix sensor - SET CALIBRATION
void ofApp::E256_setCaliration(void) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/CONTROL");
  OSCmsg.addIntArg(CONTROL_CHANGE);
  OSCmsg.addIntArg(CALIBRATE);
  serialDevice.send(OSCmsg);
};

// E256 matrix sensor - SET THRESHOLD
void ofApp::E256_setTreshold(int & tresholdValue) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/CONTROL");
  OSCmsg.addIntArg(CONTROL_CHANGE);
  OSCmsg.addIntArg(THRESHOLD);
  OSCmsg.addIntArg((int32_t)tresholdValue);
  serialDevice.send(OSCmsg);
};

// E256 matrix sensor - MATRIX DATA REQUEST
// 16*16 matrix row data request
void ofApp::E256_rawDataRequest(void) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/CONTROL");
  OSCmsg.addIntArg(CONTROL_CHANGE);
  OSCmsg.addIntArg(RAW_MATRIX);
  serialDevice.send(OSCmsg);
};

// E256 matrix sensor - MATRIX DATA REQUEST
// 64*64 matrix interpolated data request
void ofApp::E256_interpDataRequest(void) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/CONTROL");
  OSCmsg.addIntArg(CONTROL_CHANGE);
  OSCmsg.addIntArg(INTERP_MATRIX);
  serialDevice.send(OSCmsg);
};

// E256 matrix sensor - BLOBS REQUEST
void ofApp::E256_blobsRequest(void) {
  ofxOscMessage OSCmsg;
  OSCmsg.setAddress("/CONTROL");
  OSCmsg.addIntArg(CONTROL_CHANGE);
  OSCmsg.addIntArg(BLOBS_PLAY);
  serialDevice.send(OSCmsg);
};

// E256 matrix sensor - Toggle full screen mode
void ofApp::keyPressed(int key) {
  switch(key) {
    case 'f':
    ofToggleFullscreen();
    break;
    default:
    break;
  };
};

void ofApp::exit(void) {
  setCalirationButton.removeListener(this, &ofApp::E256_setCaliration);
  setTresholdSlider.removeListener(this, &ofApp::E256_setTreshold);
  getRawDataToggle.removeListener(this, &ofApp::E256_rawDataRequestStart);
  getInterpDataToggle.removeListener(this, &ofApp::E256_interpDataRequestStart);
  getBlobsToggle.removeListener(this, &ofApp::E256_blobsRequestStart);
  serialDevice.unregisterAllEvents(this);
};
