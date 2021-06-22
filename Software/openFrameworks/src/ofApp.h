/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/


#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxGui.h"

#define MIDI_PORT_NAME        "ETEXTILE_SYNTH"
#define VERSION               "1.0.5"
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

class ofApp : public ofBaseApp, public ofxMidiListener {

public:

    void                          setup();
    void                          update();
    void                          draw();
    void                          exit();

    ofxPanel                      gui;
    ofxButton                     setCalirationButton;
    ofxIntSlider                  setTresholdSlider;
    ofxToggle                     getBlobsToggle;
    ofxToggle                     getRawDataToggle;
    ofxToggle                     getInterpDataToggle;
    ofxToggle                     getBinDataToggle;

    //uint8_t                       rawFrameBuffer[RAW_FRAME];
    //uint8_t                       interpFrameBuffer[NEW_FRAME];

    //MIDI stuff
    ofMutex                       midiMutex;     

	void                          newMidiMessage(ofxMidiMessage& eventArgs);
	ofxMidiIn                     midiIn;
	std::vector<ofxMidiMessage>   midiMessages;
	std::size_t maxMessages = 10; //< max number of messages to keep track of
	ofxMidiOut                    midiOut;
	//vector<unsigned char>         sysexMsg;

    void                          E256_setCaliration();
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

    void                          E256_rawDataRequest();
    void                          E256_interpDataRequest();
    void                          E256_binDataRequest();
    void                          E256_blobsRequest();

    ofMesh                        rawDataMesh;
    ofMesh                        interpDataMesh;

    ofTrueTypeFont	              FreeSansBold;
    void                          keyPressed(int key);

};
