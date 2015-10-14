/**
 * File: ofApp.h
 * Author: Sanjay Kannan
 * ---------------------
 * Header file for the entire
 * OpenFrameworks application.
 */

#pragma once
#include <Poco/Mutex.h>
#include "ofMain.h"
#include "ofxBeat.h"

// used for flag gradient colorizing
typedef ofColor (*Gradient)(int, int, int);

struct Flag {
  string name;
  ofColor colorOne;
  ofColor colorTwo;
  ofColor colorThree;
  Gradient gradientFn;
};

class ofApp : public ofBaseApp {
  public:
    // this has to go here
    // do not ask me why
    ofxBeat beat;

    void setup();
    void update();
    void draw();

    // some usual boilerplate
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    // just like the RTAudio callback, but only for input
    void audioIn(float* input, int bufferSize, int nChannels);

  private:
    ofSoundStream soundStream;
    ofImage background;

    ofMesh wave;
    ofMesh spectrum;

    // tinker with this
    int bufferSize;
    int maxBufferCount;

    // viz parameters
    bool ellipseMode;
    bool fullscreenMode;
    bool attenuationMode;
    bool cyclingMode;
    bool captureSound;
    int drawingType = 1;
    float rotMultiplier = 10;

    // for flag cycling
    int beatCount = 0;

    // coloring parameters by flag
    const static int flagCount = 4;
    Flag flags[flagCount];
    int flagIndex = 0;
    Flag country;

    // volume tracking
    float smoothedVol;
    float scaledVol;

    // vector just for ease
    vector<float> inBuffer;
    vector<vector<float> > fftBuffers;
    Poco::Mutex bufferLock;
};
