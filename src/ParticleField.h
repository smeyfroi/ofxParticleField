#pragma once

#include "ofTexture.h"
#include "ofVboMesh.h"
#include "PingPongFbo.h"
#include "DrawShader.h"
#include "UpdateShader.h"
#include "InitShader.h"
#include "ofxGui.h"

namespace ofxParticleField {


// Accepts a field as Pixels, which are loaded into the fieldTexture,
// or an ofFbo reference, whose texture is copied into fieldFbo.
class ParticleField {
public:
  ParticleField();
  void setup(int approxNumParticles, ofFloatColor particleColor, float field1ValueOffset, float field2ValueOffset);
  void resizeParticles(int newApproxNumParticles);
  void update();
  void draw(ofFbo& foregroundFbo);
  void setField1(const ofTexture& fieldTexture);
  void setField2(const ofTexture& fieldTexture);
  
  int getParticleCount() const { return particleDataFbo.getWidth() * particleDataFbo.getHeight(); }
  
  std::string getParameterGroupName() const { return "Particle Field"; }
  ofParameterGroup parameters;
  ofParameter<float> velocityDampingParameter { "velocityDamping", 0.997, 0.99, 1.0 };
  ofParameter<float> forceMultiplierParameter { "forceMultiplier", 0.01, 0.0, 1.0 };
  ofParameter<float> maxVelocityParameter { "maxVelocity", 0.001, 0.0, 0.01 };
  ofParameter<float> particleSizeParameter { "particleSize", 2.0, 1.0, 50.0 };
  ofParameter<float> jitterStrengthParameter { "jitterStrength", 0.2, 0.0, 2.0 };
  ofParameter<float> jitterSmoothingParameter { "jitterSmoothing", 0.1, 0.0, 1.0 };
  ofParameter<float> speedThresholdParameter { "speedThreshold", 2.0, 0.1, 10.0 };
  ofParameterGroup& getParameterGroup();
  
private:
  size_t numDataBuffers = 3; // position, velocity, jitter
  PingPongFbo particleDataFbo;
  ofFboSettings createParticleDataFboSettings(size_t width, size_t height) const;
  
  ofVboMesh mesh;
  ofFloatColor particleColor;
  
  DrawShader drawShader;
  UpdateShader updateShader;
  InitShader initShader;
    
  float field1ValueOffset, field2ValueOffset; // -0.5 when values are [0,v]; 0.0 when values are [-v,v]
  ofTexture field1Texture, field2Texture;
  ofTexture emptyFieldTexture;

};



}
