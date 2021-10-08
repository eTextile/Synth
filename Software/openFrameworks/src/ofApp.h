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
#define BlobX 3  // [3] Blob X centroid position
#define BlobY 4  // [4] Blob Y centroid position
#define BlobZ 5  // [5] Blob depth
#define BlobW 6  // [6] Blob width
#define BlobH 7  // [7] Blob height

#define LINE_OUT          0
#define SIG_IN            1
#define SIG_OUT           2
#define THRESHOLD         3
#define CALIBRATE         4
#define SAVE              5
#define MIDI_BLOBS_PLAY   6
#define MIDI_BLOBS_LEARN  7
#define MIDI_MAPPING      8
#define MIDI_RAW          9
#define MIDI_INTERP       10
#define MIDI_OFF          11

struct blob_t {
  int  id;   // [0] Blob ID
  int  bx;   // [1] Blob X centroid position
  int  by;   // [2] Blob Y centroid position
  int  bz;   // [3] Blob depth
  int  bw;   // [4] Blob width
  int  bh;   // [5] Blob Height
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
    std::vector<ofxMidiMessage>   midiInputCopy;
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
