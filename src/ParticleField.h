#pragma once

#include "ofTexture.h"
#include "ofVboMesh.h"
#include "PingPongFbo.h"
#include "DrawShader.h"
#include "UpdateShader.h"

namespace ofxParticleField {



class ParticleField {
public:
  void setup(int approxNumParticles); // we calculate actual numParticles as a square number
  void update();
  void draw(ofFbo& foregroundFbo);
  void setFieldTexture(ofTexture&& tex);
  
private:
  size_t numDataBuffers = 2; // position and velocity
  PingPongFbo particleDataFbo;
  ofFboSettings createParticleDataFboSettings(size_t width, size_t height) const;
  void setupParticlePositions();
  void setupParticleVelocities();
  void loadParticleData(size_t dataIndex, const float* data);
  
  ofVboMesh mesh;
  
  DrawShader drawShader;
  UpdateShader updateShader;
  
  ofTexture fieldTexture;
  
};



}
