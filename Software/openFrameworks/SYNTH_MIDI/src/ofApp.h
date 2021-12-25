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

#define NAME                  "256"
#define PROJECT               "ETEXTILE-SYNTHESIZER"
#define VERSION               "1.0.9"

#define RAW_COLS              16
#define RAW_ROWS              16
#define RAW_FRAME             (RAW_COLS * RAW_ROWS)
#define NEW_COLS              (RAW_COLS * 4)
#define NEW_ROWS              (RAW_ROWS * 4)
#define NEW_FRAME             (NEW_COLS * NEW_ROWS)

#define LOAD_CONFIG           0  // E256-LEDs:
#define UPLOAD_CONFIG         1  // E256-LEDs:
#define CALIBRATE             2  // E256-LEDs:
#define BLOBS_PLAY            3  // Send all blobs values over USB using MIDI format
#define BLOBS_LEARN           4  // Send separate blobs values over USB using MIDI format
#define MAPPING_LIB           5  // E256-LEDs:
#define RAW_MATRIX            6
#define INTERP_MATRIX         7
#define ERROR                 6  // E256-LEDs:

#define ALL_OFF               9

#define SIG_IN                0  // E256-LEDs: | 1 | 0 |
#define SIG_OUT               1  // E256-LEDs: | 0 | 1 |
#define LINE_OUT              2  // E256-LEDs: | 0 | 0 |
#define THRESHOLD             3  // E256-LEDs: | 1 | 1 |

#define MIDI_OUTPUT_CHANNEL   1  // [1:15] Set the MIDI_OUTPUT channel

// MIDI_CONTROL_CHANGE
#define BlobX 3  // [3] Blob X centroid position
#define BlobY 4  // [4] Blob Y centroid position
#define BlobZ 5  // [5] Blob depth
#define BlobW 6  // [6] Blob width
#define BlobH 7  // [7] Blob height

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
