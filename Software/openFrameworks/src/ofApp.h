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
#define VERSION               "1.0.6"
#define RAW_COLS              16
#define RAW_ROWS              16
#define RAW_FRAME             (RAW_COLS * RAW_ROWS)
#define NEW_COLS              (RAW_COLS * 4)
#define NEW_ROWS              (RAW_ROWS * 4)
#define NEW_FRAME             (NEW_COLS * NEW_ROWS)

#define BI 0  // [0] Blob UID
#define BS 1  // [1] Blob State
#define BL 2  // [2] Blob Last State
#define BX 3  // [3] Blob X centroid position
#define BY 4  // [4] Blob Y centroid position
#define BW 5  // [5] Blob width
#define BH 6  // [6] Blob Height
#define BD 7  // [7] Blob Depth

struct blob_t {
  int8_t  id;         // [0] Blob UID
  int8_t  state;      // [1] Blob State
  int8_t  lastState;  // [2] Blob Last State
  int8_t  cx;         // [3] Blob X centroid position
  int8_t  cy;         // [4] Blob Y centroid position
  int8_t  width;      // [5] Blob width
  int8_t  height;     // [6] Blob Height
  int8_t  depth;      // [7] Blob Depth
};

class ofApp : public ofBaseApp, public ofxMidiListener {

public:

    void                          setup();
    void                          update();
    void                          draw();
    void                          exit();

    ofxPanel                      gui;
    ofxIntSlider                  setTresholdSlider;
    ofxButton                     setCalirationButton;
    ofxToggle                     getRawToggle;
    ofxToggle                     getInterpToggle;
    ofxToggle                     setMidiLearnToggle;
    ofxToggle                     getBlobsToggle;

    ofMutex                       midiMutex;
	  void                          newMidiMessage(ofxMidiMessage& eventArgs);
    ofxMidiIn                     midiIn;
    ofxMidiOut                    midiOut;
    std::vector<ofxMidiMessage>   midiMessages;
    std::vector<unsigned char>    sysexMsg;
    std::size_t maxMessages = 128;
    std::vector<blob_t>           blobs;

    void                          E256_setTreshold(int & sliderValue);
    void                          E256_setCaliration();
    void                          E256_setMidiLearn(bool & val);
    void                          E256_getRaw(bool & val);
    void                          E256_getInterp(bool & val);
    void                          E256_getBlobs(bool & val);

    bool                          getRaw;
    bool                          getInterp;
    bool                          midiLearn;
    bool                          getBlobs;

    ofMesh                        rawDataMesh;
    ofMesh                        interpDataMesh;

    ofTrueTypeFont	              FreeSansBold;
    void                          keyPressed(int key);

};
