#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOscSerial.h"

#define NAME                     "E256"
#define PROJECT                  "ETEXTILE-SYNTHESIZER"
#define VERSION                  "1.0.8"

#define USB_PORT              "/dev/ttyACM0"
#define BAUD_RATE             230400  // With Teensy, it's always the same native speed. The baud rate setting is ignored.
#define RAW_COLS              16
#define RAW_ROWS              16
#define DUAL_ROWS             (RAW_ROWS / 2)
#define SCALE_X               4
#define SCALE_Y               4
#define RAW_FRAME             (RAW_COLS * RAW_ROWS)
#define NEW_COLS              (RAW_COLS * SCALE_X)
#define NEW_ROWS              (RAW_ROWS * SCALE_Y)
#define NEW_FRAME             (NEW_COLS * NEW_ROWS)

#define LINE_OUT              0 // LED | 0 | 0 |
#define SIG_IN                1 // LED | 1 | 0 |
#define SIG_OUT               2 // LED | 0 | 1 |
#define THRESHOLD             3 // LED | 1 | 1 |
#define CALIBRATE             4
#define SAVE                  5
#define BLOBS_PLAY            6
#define BLOBS_LEARN           7
#define BLOBS_MAPPING         8
#define MATRIX_MODE_RAW       9
#define MATRIX_MODE_INTERP    10
#define ALL_OFF               11

#define CONTROL_CHANGE        176 //0xB0
#define NOTE_ON               144 //0x90
#define NOTE_OFF              128 //0x80

#define HOST                  "localhost"
//#define HOST                "192.168.0.101"
#define UDP_OUTPUT_PORT       7771

struct blob_t {
  int  id;   // [0] Blob ID
  int  bx;   // [1] Blob X centroid position
  int  by;   // [2] Blob Y centroid position
  int  bz;   // [3] Blob depth
  int  bw;   // [4] Blob width
  int  bh;   // [5] Blob Height
};

using namespace ofxIO;

class ofApp: public ofBaseApp {

public:
    void                          setup(void);
    void                          update(void);
    void                          draw(void);
    void                          exit(void);


    void                          onOscMessage(const ofxOscMessage& message);
    void                          onSerialError(const ofxIO::SerialBufferErrorEventArgs& error);

    OSCSerialDevice               serialDevice;
    std::vector<ofxOscMessage>    OSCMessages;
    std::vector<blob_t>           blobs;
    //std::vector<ofboxPrimitive> boxe;

    ofxPanel                      gui;
    ofxButton                     setCalirationButton; // Button to calibrate E256
    ofxIntSlider                  setTresholdSlider; // Set E256 threshold value
    ofxToggle                     getRawDataToggle;
    ofxToggle                     getInterpDataToggle;
    ofxToggle                     getBlobsToggle;
    uint8_t                       mode;

    void                          E256_setCaliration(void);
    void                          E256_setTreshold(int & sliderValue);

    bool                          getRawData;
    bool                          getInterpData;
    bool                          getBlobs;

    void                          E256_rawDataRequestStart(bool & val);
    void                          E256_interpDataRequestStart(bool & val);
    void                          E256_blobsRequestStart(bool & val);

    void                          E256_rawDataRequest(void);
    void                          E256_interpDataRequest(void);
    void                          E256_blobsRequest(void);

    ofMesh                        rawDataMesh;
    ofMesh                        interpDataMesh;
    ofTrueTypeFont	              FreeSansBold;

    void                          keyPressed(int key);

  };
