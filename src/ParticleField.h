#pragma once

#include <functional>
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
  void setup(ofFloatColor particleColor, float field1ValueOffset, float field2ValueOffset);
  void resizeParticles(int newApproxNumParticles);
  void update();
  float smallParticleSize() const { return std::min(particleSizeParameter / 12.0f, 1.0f); }
  void draw(ofFbo& foregroundFbo, bool smallParticles = false); // smallParticles uses smallParticleSize
  void setField1(const ofTexture& fieldTexture);
  void setField2(const ofTexture& fieldTexture);
  void updateRandomColorBlocks(int numBlocks, int blockSize, std::function<ofFloatColor(size_t)> colorFunc);
  
  int getParticleCount() const { return particleDataFbo.getWidth() * particleDataFbo.getHeight(); }
  
  std::string getParameterGroupName() const { return "Particle Field"; }
  ofParameterGroup parameters;
  ofParameter<float> ln2ParticleCountParameter { "ln2ParticleCount", 14.0, 10.0, 22.0 };
  ofParameter<float> velocityDampingParameter { "velocityDamping", 0.997, 0.99, 1.0 };
  ofParameter<float> forceMultiplierParameter { "forceMultiplier", 0.01, 0.0, 1.0 };
  ofParameter<float> maxVelocityParameter { "maxVelocity", 0.001, 0.0, 0.01 };
  ofParameter<float> particleSizeParameter { "particleSize", 8.0, 1.0, 50.0 };
  ofParameter<float> jitterStrengthParameter { "jitterStrength", 0.2, 0.0, 2.0 };
  ofParameter<float> jitterSmoothingParameter { "jitterSmoothing", 0.1, 0.0, 1.0 };
  ofParameter<float> speedThresholdParameter { "speedThreshold", 2.0, 0.1, 10.0 };
  ofParameter<float> minWeightParameter { "minWeight", 0.5, 0.1, 2.0 };
  ofParameter<float> maxWeightParameter { "maxWeight", 2.0, 0.5, 10.0 };
  ofParameterGroup& getParameterGroup();
  
private:
  size_t numDataBuffers = 4; // position, velocity, jitter, weight
  PingPongFbo particleDataFbo;
  ofFboSettings createParticleDataFboSettings(size_t width, size_t height) const;
  void rebuildMesh(size_t width, size_t height);
  void calculateParticleDimensions(int approxNumParticles, size_t& outWidth, size_t& outHeight) const;
  void initializeParticleRegion(size_t x, size_t y, size_t width, size_t height);
  void onLn2ParticleCountChanged(float& value);
  
  bool pendingResize = false;
  int pendingParticleCount = 0;
  float lastResizeTime = 0;
  float resizeDebounceDelay = 0.3f;
  
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
