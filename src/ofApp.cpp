/**
 * File: ofApp.cpp
 * Author: Sanjay Kannan
 * ---------------------
 * Implements an audio waveform and spectrum
 * visualizer with basic beat detection.
 */

#include <fftw3.h>
#include <math.h>
#include <stdlib.h>
#include "ofApp.h"

// goes outward from a bright red to a bright blue
ofColor colorAmerica(int index, int maxIndex, int alpha) {
  float fraction = (float) index / (float) maxIndex;
  int red = 200 * fraction; // increasing
  int green = 120 * (1 - fraction); // decreasing
  int blue = 240 * (1 - fraction); // decreasing
  return ofColor(red, green, blue, alpha);
}

// goes outward from a bright green to a solid white
ofColor colorIndia(int index, int maxIndex, int alpha) {
  float fraction = (float) index / (float) maxIndex;
  int red = 255 - 236 * fraction; // decreasing
  int green = 255 - 119 * fraction; // decreasing
  int blue = 255 - 247 * fraction; // decreasing
  return ofColor(red, green, blue, alpha);
}

// goes outward from a Greek blue to a solid white
ofColor colorGreece(int index, int maxIndex, int alpha) {
  float fraction = (float) index / (float) maxIndex;
  int red = 255 - 242 * fraction; // decreasing
  int green = 255 - 161 * fraction; // decreasing
  int blue = 255 - 80 * fraction; // decreasing
  return ofColor(red, green, blue, alpha);
}

// goes outward from a dark red to a grassy green
ofColor colorMexico(int index, int maxIndex, int alpha) {
  float fraction = (float) index / (float) maxIndex;
  int red = 0 + 206 * fraction; // increasing
  int green = 104 - 87 * fraction; // decreasing
  int blue = 71 - 33 * fraction; // decreasing
  return ofColor(red, green, blue, alpha);
}

/**
 * Function: setup
 * ---------------
 * Initializes audio capture and a bunch
 * of variables related to rendering.
 */
void ofApp::setup() {
  // initialization code here borrowed
  // and modified from the OpenFrameworks
  // audio input example

  ofSetVerticalSync(true);
  ofSetCircleResolution(80);
  ofBackground(54, 54, 54);

  // if you want to set a different device id
  // soundStream.listDevices();
  // soundStream.setDeviceID(0);

  bufferSize = 512;
  maxBufferCount = 80;
  inBuffer.assign(bufferSize, 0.0);

  // volume tracking
  smoothedVol = 0.0;
  scaledVol = 0.0;

  // 0 output channels
  // 2 input channels
  // 44100 samples per second
  // 1024 samples per buffer
  // 4 buffers to queue

  soundStream.setup(this, 0, 2, 44100, bufferSize, 4);
  // background.loadImage("ocean.jpg");

  // initialize flag based color schemes [see above for descriptions of the gradients]
  flags[0] = { "America", ofColor(255, 255, 255), ofColor(255, 255, 255), ofColor(255, 0, 0), &colorAmerica };
  flags[1] = { "India", ofColor(255, 255, 255), ofColor(255, 153, 51), ofColor(19, 136, 8), &colorIndia };
  flags[2] = { "Greece", ofColor(245, 245, 220), ofColor(13, 94, 175), ofColor(255, 255, 255), &colorGreece };
  flags[3] = { "Mexico", ofColor(255, 255, 255), ofColor(0, 104, 71), ofColor(206, 17, 38), &colorMexico };

  // ellipses are ugly
  ellipseMode = false;
  attenuationMode = false;
  cyclingMode = true;
  captureSound = true;
  fullscreenMode = false;
  country = flags[flagIndex];
}

/**
 * Function: update
 * ----------------
 * See note below.
 */
void ofApp::update() {
  // TODO: maybe move some of the
  // audio in code here. although
  // that will affect the appearance
  // due to the way audio is read in
}

/**
 * Function: draw
 * --------------
 * Use the waveform and FFT buffers
 * to render a tunnel-like thing.
 */
void ofApp::draw() {
  // TBD: figure out good title
  ofSetWindowTitle(country.name);

  float time = ofGetElapsedTimef(); // get time in seconds
  float angle = time * rotMultiplier; // rotation angle [and speed]

  /* float widthAngle = angle / 180 * PI;
  float heightAngle = (angle - 90) / 180 / PI;
  float scaledWidth, scaledHeight;

  float absCosWidthAngle = fabs(cos(widthAngle));
  float absSinWidthAngle = fabs(sin(widthAngle));
  float absCosHeightAngle = fabs(cos(heightAngle));
  float absSinHeightAngle = fabs(sin(heightAngle));

  if (ofGetWidth() / 2 * absSinWidthAngle <= ofGetHeight() / 2 * absCosWidthAngle)
    scaledWidth = ofGetWidth() / 2 / absCosWidthAngle;
  else scaledWidth = ofGetHeight() / 2 / absSinWidthAngle;

  if (ofGetWidth() / 2 * absSinHeightAngle <= ofGetHeight() / 2 * absCosHeightAngle)
    scaledHeight = ofGetWidth() / 2 / absCosHeightAngle;
  else scaledHeight = ofGetHeight() / 2 / absSinHeightAngle; */

  // just some experimentation for a seamless tunnel
  float scaledHeight = max(ofGetHeight(), ofGetWidth());
  float scaledWidth = max(ofGetHeight(), ofGetWidth());

  // just for correctness
  bufferLock.lock();

  // calculate the waveform drawing
  for (int i = 0; i < bufferSize; i += 2) {
    float offset = (scaledWidth / 2) / (bufferSize / 2) * bufferSize / 12;
    float x = (scaledWidth / 2) / (bufferSize / 2) * i; // fit hallway
    float y = inBuffer[i] * (scaledHeight / 2) / (bufferSize / 2) * i;
    wave.addVertex(ofPoint(x + offset, y, 0));

    float alpha = ((float) 255 / (float) bufferSize) * 2 * i;
    ofColor waveColor = country.colorOne; waveColor.a = (int) alpha;
    wave.addColor(waveColor);
  }

  // so threadsafe
  bufferLock.unlock();

  float ellipseWidth = ofGetWidth() / 2 * sqrt(2);
  float ellipseHeight = ofGetHeight() / 2 * sqrt(2);
  if (fftBuffers.size() <= 10) return; // get some data first

  if (beat.isHat() || beat.isSnare())
    beatCount += 1; // for cycling

  if (beatCount % 64 == 0 && cyclingMode) // cycle flag
    country = flags[(++flagIndex) % flagCount];

  // calculate the FFT spectrum drawing
  for (int i = 0; i < fftBuffers.size() - 10; i += 1) {
    vector<float> fftBuffer = fftBuffers[i];
    int rev = fftBuffers.size() - 1 - i; // see above

    // so that we avoid overlap on the innermost rings
    int step = (rev > 10) ? 2 : ((rev > 5) ? 4 : 8);

    // for each FFT frequency in the buffer [by step]
    for (int j = 0; j < bufferSize; j += step) {
      float magnitude = sqrt(pow(fftBuffer[j], 2) + pow(fftBuffer[j + 1], 2)); // complex norm
      float theta = 2 * PI / (bufferSize / step) * j / step; // parametrize frequency by angle

      // used for ellipse parametrization in ellipse mode
      float maxRadius = sqrt(pow(ellipseWidth * cos(theta), 2) + pow(ellipseHeight * sin(theta), 2));
      float bufferRadius = maxRadius / fftBuffers.size() * rev; // get radius by buffer num and angle

      float maxWidth = scaledWidth / 2;
      float maxHeight = scaledHeight / 2;
      float bufferWidth = maxWidth / (fftBuffers.size() - 15) * rev;
      float bufferHeight = maxHeight / (fftBuffers.size() - 15) * rev;

      // two ways
      float x, y;

      if (ellipseMode) x = (1 + magnitude) * bufferRadius * cos(theta); // for rectangles we use Lame curves
      else x = (1 + magnitude) * bufferWidth * (abs(cos(theta)) * cos(theta) + abs(sin(theta)) * sin(theta));

      if (ellipseMode) y = (1 + magnitude) * bufferRadius * sin(theta); // for rectangles we use Lame curves
      else y = (1 + magnitude) * bufferHeight * (abs(cos(theta)) * cos(theta) - abs(sin(theta)) * sin(theta));

      spectrum.addVertex(ofPoint(x, y, 0)); // TBD: depth?
      float alpha = ((float) 20 / (float) fftBuffers.size()) * rev + 235;

      scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
      // if (beat.isKick()) spectrum.addColor(country.colorOne);
      if (beat.isHat()) spectrum.addColor(ofColor(country.colorTwo));
      if (beat.isSnare()) spectrum.addColor(ofColor(country.colorThree));
      spectrum.addColor(country.gradientFn(i, fftBuffers.size(), alpha));
    }
  }

  ofEnableDepthTest(); // enable z-buffering
  ofBackgroundGradient(ofColor(0, 0, 0), ofColor(0, 0, 0));

  ofPushMatrix(); // draw the spectrum first
    ofEnableAlphaBlending(); // for transparency
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
    ofRotate(angle); // see controls

    if (drawingType == 1) {
      spectrum.setMode(OF_PRIMITIVE_LINE_STRIP);
      ofSetLineWidth(2);
    }

    else if (drawingType == 2) {
      spectrum.setMode(OF_PRIMITIVE_POINTS);
      glPointSize(3); glEnable( GL_POINT_SMOOTH );
    }

    // drawingType == 3 corresponds to triangles
    else spectrum.setMode(OF_PRIMITIVE_TRIANGLES);

    spectrum.draw(); // draw the spectrum mesh
    ofDisableAlphaBlending(); // turn off transparency
    spectrum.clear(); // clear the mesh for next time
  ofPopMatrix(); // restore the coordinate system

  ofPushMatrix(); // then draw the waveform
    ofEnableAlphaBlending(); // for transparency
    // ofSetColor(ofColor(255, 255, 255));

    wave.setMode(OF_PRIMITIVE_LINE_STRIP);
    ofSetLineWidth(5);

    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
    ofRotate(angle); // sync with spectrum rotation
    wave.draw(); // draw the mesh
    ofRotate(180); // reflect mesh
    wave.draw(); // draw mesh again
    ofDisableAlphaBlending();

    wave.clear(); // clear the mesh for next time
  ofPopMatrix(); // restore the coordinate system
}

/**
 * Function: audioIn
 * -----------------
 * Capture audio and process it through
 * beat detection as well as an FFT.
 */
void ofApp::audioIn(float* input, int bufferSize, int nChannels) {
  if (!captureSound) return; // sound capture is currently paused

	// store possibly stereo audio in a mono float buffer
	bufferLock.lock(); // just for correctness

  // update FFT beat detection bins with new data
  beat.audioReceived(input, bufferSize, nChannels);
  beat.update(ofGetElapsedTimeMillis());

	for (int i = 0; i < bufferSize; i += 1) {
    inBuffer[i] = 0; // initialize this

    for (int j = 0; j < nChannels; j += 1) {
      // signal pickup threshold value [ignore ambient noise]
      if (attenuationMode && fabs(input[i * 2 + j]) < 0.05) {
        inBuffer[i] += 0;
        continue;
      }

      // multiply by 1 / nChannels to convert to mono
  		inBuffer[i] += input[i * 2 + j] / nChannels;
    }
  }

  // copy the input buffer to FFT storage
  float* fftSamples = new float[bufferSize];
  float* fftWindow = new float[bufferSize];
  copy(inBuffer.begin(), inBuffer.end(), fftSamples);
  bufferLock.unlock(); // once copy is finished

  // execute a complex FFT on a buffer of real numbers, leaving two numbers for bufferSize / 2 frequencies
  fftwf_plan p = fftwf_plan_dft_r2c_1d(bufferSize, fftSamples, (fftwf_complex*) fftSamples, FFTW_ESTIMATE);
  fftwf_execute(p); // repeat as needed
  fftwf_destroy_plan(p);

  // delete old fftBuffers as we go
  if (fftBuffers.size() > maxBufferCount)
    fftBuffers.erase(fftBuffers.begin());

  fftBuffers.push_back(vector<float>(bufferSize));
  vector<float>& fftBuffer = fftBuffers[fftBuffers.size() - 1];
  copy(fftSamples, fftSamples + bufferSize, fftBuffer.begin());

  // deallocate memory
  delete fftSamples;
  delete fftWindow;
}

/**
 * Function: keyPressed
 * --------------------
 * Handle key presses.
 */
void ofApp::keyPressed(int key) {
  if (key == 'q') rotMultiplier > 0 ? rotMultiplier-- : rotMultiplier;
  if (key == 'w') rotMultiplier = 10; // return multiplier to default
  if (key == 'e') rotMultiplier < 500 ? rotMultiplier++ : rotMultiplier;
  if (key == 'a') attenuationMode = !attenuationMode;
	if (key == 's') captureSound = !captureSound; // toggle drawing
  if (key == 'd') country = flags[++flagIndex % flagCount];
  if (key == 'f') ofSetFullscreen(fullscreenMode = !fullscreenMode);
  if (key == 'z') drawingType = 1; // connected lines
  if (key == 'x') drawingType = 2; // disjoint points
  if (key == 'c') drawingType = 3; // weird triangles
  if (key == 'v') cyclingMode = !cyclingMode;

  // revert all of these
  // settings to defaults
  if (key == 'r') {
    rotMultiplier = 10; // qwe
    ellipseMode = false; // none
    attenuationMode = false; // a
    captureSound = true; // s
    country = flags[0]; // d
    flagIndex = 0; // also d
    fullscreenMode = true; // f
    ofSetFullscreen(true); // also f
    drawingType = 1; // zxc
    cyclingMode = true; // v
  }
}

/**
 * Function: keyReleased
 * ---------------------
 * Handle key releases.
 */
void ofApp::keyReleased(int key) {
  // nothing so far with this
  // TODO: any ideas for this
}

/**
 * Function: mouseMoved
 * --------------------
 * Handle mouse movements.
 */
void ofApp::mouseMoved(int x, int y) {
  // nothing so far with this
  // TODO: any ideas for this
}

/**
 * Function: mouseDragged
 * ----------------------
 * Handle mouse dragging.
 */
void ofApp::mouseDragged(int x, int y, int button) {
  // nothing so far with this
  // TODO: any ideas for this
}

/**
 * Function: mousePressed
 * ----------------------
 * Handle mouse presses.
 */
void ofApp::mousePressed(int x, int y, int button) {
  // nothing so far with this
  // TODO: any ideas for this
}

/**
 * Function: mouseReleased
 * -----------------------
 * Handle mouse releases.
 */
void ofApp::mouseReleased(int x, int y, int button) {
  // nothing so far with this
  // TODO: any ideas for this
}

/**
 * Function: windowResized
 * -----------------------
 * Handle window resizing.
 */
void ofApp::windowResized(int w, int h) {
  // nothing so far with this
  // resizing is implicitly handled
  // TODO: any ideas for this
}

/**
 * Function: gotMessage
 * --------------------
 * Handle messages.
 */
void ofApp::gotMessage(ofMessage msg) {
  // nothing so far with this
  // TODO: any ideas for this
}

/**
 * Function: dragEvent
 * -------------------
 * Handle mouse dragging.
 */
void ofApp::dragEvent(ofDragInfo dragInfo) {
  // nothing so far with this
  // TODO: any ideas for this
}
