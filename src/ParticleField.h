#pragma once

#include "ofTexture.h"
#include "ofVboMesh.h"
#include "PingPongFbo.h"
#include "DrawShader.h"
#include "UpdateShader.h"
#include "ofxGui.h"

namespace ofxParticleField {


// Accepts a field as Pixels, which are loaded into the fieldTexture,
// or an ofFbo reference, whose texture is copied into fieldFbo.
class ParticleField {
public:
  void setup(int approxNumParticles, ofFloatColor particleColor, float fieldValueOffset); // we calculate actual numParticles as a rectangular number
  void update();
  void draw(ofFbo& foregroundFbo);
  void setField(const ofFbo& fieldFbo);
  
  std::string getParameterGroupName() const { return "Particle Field"; }
  ofParameterGroup parameters;
  ofParameter<float> velocityDampingParameter { "velocityDamping", 0.997, 0.99, 1.0 };
  ofParameter<float> forceMultiplierParameter { "forceMultiplier", 0.01, 0.0, 1.0 };
  ofParameter<float> maxVelocityParameter { "maxVelocity", 0.001, 0.0, 0.01 };
  ofParameter<float> particleSizeParameter { "particleSize", 2.0, 1.0, 50.0 };
  ofParameterGroup& getParameterGroup();

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
  
  float fieldValueOffset; // -0.5 when values are [0,v]; 0.0 when values are [-v,v]
  
  ofFbo fieldFbo; // alternative to fieldTexture
};



}
