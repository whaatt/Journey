/**
 * File: main.cpp
 * Author: Sanjay Kannan
 * ---------------------
 * Initializes OpenFrameworks
 * and runs the windowed app.
 */

#include "ofMain.h"
#include "ofApp.h"

/**
 * Function: main
 * --------------
 * Sets up OpenFrameworks
 * and runs the window thread.
 */
int main() {
  // set up the OpenGL context in window
  ofSetupOpenGL(1024, 768, OF_WINDOW);

  cout << "Journey To The Motherland" << endl;
  cout << "By Sanjay Kannan for CS 256A" << endl;
  cout << "Last Updated: 13 October 2015" << endl;
  cout << endl;
  cout << "Press [q] to reduce rotation." << endl;
  cout << "Press [w] for default rotation." << endl;
  cout << "Press [e] to increase rotation." << endl;
  cout << "Press [r] to revert to defaults." << endl;
  cout << "Press [a] to toggle attenuation." << endl;
  cout << "Press [s] to toggle sound stream." << endl;
  cout << "Press [d] to change country." << endl;
  cout << "Press [f] to toggle fullscreen." << endl;
  cout << "Press [z] to draw spectrum lines." << endl;
  cout << "Press [x] to draw spectrum points." << endl;
  cout << "Press [c] to draw spectrum triangles." << endl;
  cout << "Press [v] to turn off flag changes." << endl;

  // this kicks off the running of my app
  // can be OF_WINDOW or OF_FULLSCREEN
  // pass in width and height too:
  ofRunApp(new ofApp());
}
