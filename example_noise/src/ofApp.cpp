#include "ofApp.h"


static ofTexture makePerlin2DNoiseTexture(int w, int h, float scale) {
  ofFloatPixels pixels;
  pixels.allocate(w, h, OF_PIXELS_RGB);
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      float n1 = ofNoise(x * scale, y * scale);
      float n2 = ofNoise((x+5000) * scale, (y+5000) * scale);
      pixels.setColor(x, y, ofFloatColor(n1, n2, 0.0, 0.0));
    }
  }
  ofTexture texture;
  texture.allocate(pixels);
  texture.loadData(pixels);
  return texture;
}


//--------------------------------------------------------------
void ofApp::setup(){
  ofBackground(0);
  ofSetFrameRate(60);
  
  particleField.setup(100'000);
  
  fieldTexture = makePerlin2DNoiseTexture(200, 200, 0.01);
  particleField.setFieldTexture(std::move(fieldTexture));
  
  foregroundFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
  foregroundFbo.begin();
  ofClear(0, 0, 0, 255);
  foregroundFbo.end();
}

//--------------------------------------------------------------
void ofApp::update(){
  particleField.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
  particleField.draw(foregroundFbo);

  ofSetColor(255);
  foregroundFbo.draw(0.0, 0.0, ofGetWindowWidth(), ofGetWindowHeight());

  ofSetColor(255);
  ofDrawBitmapString(ofToString(ofGetFrameRate()) + " FPS", 15, 15);
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
