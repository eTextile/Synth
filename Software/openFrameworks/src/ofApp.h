/*
  **eTextile-Synthesizer**
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#pragma once

#include "ofMain.h"  //
#include "ofxMidi.h" // https://github.com/danomatika/ofxMidi
#include "ofxGui.h"  //

#define POROJECT_NAME         "ETEXTILE-SYNTH"
#define MIDI_PORT_NAME        "E256"
#define VERSION               "1.0.7"
#define RAW_COLS              16
#define RAW_ROWS              16
#define RAW_FRAME             (RAW_COLS * RAW_ROWS)
#define NEW_COLS              (RAW_COLS * 4)
#define NEW_ROWS              (RAW_ROWS * 4)
#define NEW_FRAME             (NEW_COLS * NEW_ROWS)

// MIDI_CONTROL_CHANGE
#define BI_ 0  // [0] Blob UID
#define BS_ 1  // [1] Blob State
#define BL_ 2  // [2] Blob Last State
#define BX_ 3  // [3] Blob X centroid position
#define BY_ 4  // [4] Blob Y centroid position
#define BW_ 5  // [5] Blob width
#define BH_ 6  // [6] Blob Height
#define BD_ 7  // [7] Blob Depth

#define LINE_OUT          0
#define SIG_IN            1
#define SIG_OUT           2
#define THRESHOLD         3
#define CALIBRATE         4
#define SAVE              5
#define MIDI_BLOBS_PLAY   6
#define MIDI_BLOBS_LEARN  7

#define MIDI_RAW          8
#define MIDI_INTERP       9
#define MIDI_MAPPING      10
#define MIDI_OFF          11

struct blob_t {
  uint8_t  id;   // [0] Blob ID
  uint8_t  bx;   // [1] Blob X centroid position
  uint8_t  by;   // [2] Blob Y centroid position
  uint8_t  bw;   // [3] Blob width
  uint8_t  bh;   // [4] Blob Height
  uint8_t  bd;   // [6] Blob Depth
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
    ofxToggle                     getBlobsToggle;

    ofMutex                       midiMutex;
	  void                          newMidiMessage(ofxMidiMessage& eventArgs);
    ofxMidiIn                     midiIn;
    ofxMidiOut                    midiOut;

    std::vector<ofxMidiMessage>   midiInput;
    std::vector<ofxMidiMessage>   midiCopy;
    std::vector<blob_t>           blobs;

    uint8_t                       mode;
    uint8_t                       lastMode;

    void                          E256_setTreshold(int & sliderValue);
    void                          E256_setCaliration();
    void                          E256_getRaw(bool & val);
    void                          E256_getBlobs(bool & val);
    void                          E256_setMidiLearn(bool & val);

    bool                          getRaw;
    bool                          midiLearn;
    bool                          getBlobs;

    ofMesh                        rawDataMesh;
    ofMesh                        interpDataMesh;

    ofTrueTypeFont	              FreeSansBold;
    void                          keyPressed(int key);

};
