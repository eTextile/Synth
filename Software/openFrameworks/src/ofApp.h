#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxMidi.h"

#define MIDI_PORT_NAME        "ETEXTILE_SYNTH"
#define RAW_COLS              16
#define RAW_ROWS              16
#define DUAL_ROWS             (RAW_ROWS / 2)
#define SCALE_X               4
#define SCALE_Y               4
#define RAW_FRAME             (RAW_COLS * RAW_ROWS)
#define NEW_COLS              (RAW_COLS * SCALE_X)
#define NEW_ROWS              (RAW_ROWS * SCALE_Y)
#define NEW_FRAME             (NEW_COLS * NEW_ROWS)

#define OUT_BUFFER_SIZE       1024
#define IN_BUFFER_SIZE        65535

struct blob {
  uint8_t UID;
  uint8_t state;
  uint8_t lastState;
  float Xcentroid;
  float Ycentroid;
  uint8_t boxW;
  uint8_t boxH;
  uint8_t boxD;
};



class ofApp: public ofBaseApp {

public:
    void                          setup(void);
    void                          update(void);
    void                          draw(void);
    void                          exit(void);

    ofxPanel                      gui;
    ofxButton                     setCalirationButton; // Button to calibrate E256
    ofxIntSlider                  setTresholdSlider;   // Set E256 threshold value
    ofxToggle                     getBlobsToggle;
    ofxToggle                     getRawDataToggle;
    ofxToggle                     getInterpDataToggle;
    ofxToggle                     getBinDataToggle;

    uint8_t                       rawFrameBuffer[RAW_FRAME];
    uint8_t                       interpFrameBuffer[NEW_FRAME];

    //MIDI stuff
    void newMidiMessage(ofxMidiMessage& eventArgs);
	ofxMidiIn midiIn;
	std::vector<ofxMidiMessage> midiMessages;


    void                          E256_setCaliration(void);
    void                          E256_setTreshold(int & sliderValue);

    bool                          getRawData;
    bool                          getInterpData;
    bool                          getBinData;
    bool                          getBlobs;

    void                          E256_rawDataRequestStart(bool & val);
    void                          E256_interpDataRequestStart(bool & val);
    void                          E256_binDataRequestStart(bool & val);
    void                          E256_blobsRequestStart(bool & val);

    bool                          E256_dataRequest;
    void                          E256_rawDataRequest(void);
    void                          E256_interpDataRequest(void);
    void                          E256_binDataRequest(void);
    void                          E256_blobsRequest(void);

    ofMesh                        rawDataMesh;
    ofMesh                        interpDataMesh;

    ofTrueTypeFont	              FreeSansBold;
    void                          keyPressed(int key);
    ofxOscSender                  sender;
    //std::vector<ofboxPrimitive>  boxe;
};
