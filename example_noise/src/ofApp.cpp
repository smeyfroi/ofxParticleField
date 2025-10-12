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
  ofEnableAlphaBlending();
  ofDisableArbTex();

  ofBackground(0);
  ofSetFrameRate(30);
  glEnable(GL_PROGRAM_POINT_SIZE);
  
  particleField.setup(ofFloatColor(0.5, 0.3, 1.0, 0.7), -0.5, -0.5);
  
  foregroundFbo.allocate(ofGetWidth()*2.0, ofGetHeight()*2.0, GL_RGBA);
  foregroundFbo.begin();
  ofClear(0, 0, 0, 255);
  foregroundFbo.end();

  field1Texture.allocate(fieldWidth, fieldHeight, GL_RG16F);
  field2Texture.allocate(fieldWidth, fieldHeight, GL_RG16F);

  gui.setup(particleField.getParameterGroup());
}

//--------------------------------------------------------------
void ofApp::update(){
  ofFloatPixels pixels1 = makePerlin2DNoise(fieldWidth, fieldHeight, 0.001, ofGetElapsedTimef()*0.1);
  field1Texture.loadData(pixels1);
  ofFloatPixels pixels2 = makePerlin2DNoise(fieldWidth, fieldHeight, 0.01, -1000.0 + ofGetElapsedTimef()*0.2);
  field2Texture.loadData(pixels2);

  particleField.setField1(field1Texture);
  particleField.setField2(field2Texture);
  particleField.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
  foregroundFbo.begin();
  ofClear(0, 0);
  foregroundFbo.end();
  ofEnableBlendMode(OF_BLENDMODE_ADD);
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
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

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
