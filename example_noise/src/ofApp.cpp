#include "ofApp.h"


static ofFloatPixels makePerlin2DNoise(int w, int h, float scale, float z) {
  ofFloatPixels pixels;
  pixels.allocate(w, h, OF_PIXELS_RGB);
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      float n1 = ofNoise(x * scale, y * scale, z);
      float n2 = ofNoise((x+5000) * scale, (y+5000) * scale, z);
      pixels.setColor(x, y, ofFloatColor(n1, n2, 0.0, 0.0));
    }
  }
  return pixels;
}


//--------------------------------------------------------------
void ofApp::setup(){
  ofBackground(0);
  ofSetFrameRate(30);
  glEnable(GL_PROGRAM_POINT_SIZE);
  
  particleField.setup(500'000, ofFloatColor(0.5, 0.3, 1.0, 0.7));
  
  foregroundFbo.allocate(ofGetWidth()*2.0, ofGetHeight()*2.0, GL_RGBA);
  foregroundFbo.begin();
  ofClear(0, 0, 0, 255);
  foregroundFbo.end();
  
  gui.setup(particleField.getParameterGroup());
}

//--------------------------------------------------------------
void ofApp::update(){
  particleField.setFieldTexture(makePerlin2DNoise(fieldWidth, fieldHeight, 0.01, ofGetElapsedTimef() * 0.1));
  particleField.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
  particleField.draw(foregroundFbo);

  ofSetColor(255);
  ofEnableBlendMode(OF_BLENDMODE_ALPHA);
  foregroundFbo.draw(0.0, 0.0, ofGetWindowWidth(), ofGetWindowHeight());

  ofSetColor(255);
  ofDrawBitmapString(ofToString(ofGetFrameRate()) + " FPS", 400, 15);
  
  gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
