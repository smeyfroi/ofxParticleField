#pragma once

#include "ofMain.h"
#include "ofxParticleField.h"
#include "ofxGui.h"

class ofApp: public ofBaseApp {
public:
  void setup();
  void update();
  void draw();
  
  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseScrolled(int x, int y, float scrollX, float scrollY);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

private:
  ofFbo foregroundFbo;
  
  int fieldWidth = 200;
  int fieldHeight = 200;
  ofTexture field1Texture, field2Texture;
  
  int numParticles;
  ofxParticleField::ParticleField particleField;
  
  ofxPanel gui;
};
