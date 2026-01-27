#pragma once

#include <functional>
#include <optional>

#include "DrawShader.h"
#include "InitShader.h"
#include "PingPongFbo.h"
#include "UpdateShader.h"
#include "ofMain.h"

namespace ofxParticleField {


// Accepts a field as Pixels, which are loaded into the fieldTexture,
// or an ofFbo reference, whose texture is copied into fieldFbo.
class ParticleField {
public:
  struct ParameterOverrides {
    std::optional<float> velocityDamping;
    std::optional<float> forceMultiplier;
    std::optional<float> maxVelocity;
    std::optional<float> particleSize;
    std::optional<float> jitterStrength;
    std::optional<float> jitterSmoothing;
    std::optional<float> speedThreshold;
    std::optional<float> minWeight;
    std::optional<float> maxWeight;
    std::optional<float> field1Multiplier;
    std::optional<float> field2Multiplier;
  };

  ParticleField();
  void setup(ofFloatColor particleColor, float field1ValueOffset, float field2ValueOffset);

  void setParameterOverrides(const ParameterOverrides& overrides);
  void clearParameterOverrides();

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
  ofParameter<float> ln2ParticleCountParameter { "ln2ParticleCount", 14.0, 10.0, 18.0 }; // 2^18 = 262K
  ofParameter<float> velocityDampingParameter { "velocityDamping", 0.997, 0.99, 1.0 };
  ofParameter<float> forceMultiplierParameter { "forceMultiplier", 1.0, 0.0, 2.0 };
  ofParameter<float> maxVelocityParameter { "maxVelocity", 0.001, 0.0, 0.003 };
  ofParameter<float> particleSizeParameter { "particleSize", 8.0, 1.0, 16.0 };
  ofParameter<float> jitterStrengthParameter { "jitterStrength", 0.01, 0.0, 0.05 };
  ofParameter<float> jitterSmoothingParameter { "jitterSmoothing", 0.1, 0.0, 1.0 };
  ofParameter<float> speedThresholdParameter { "speedThreshold", 2.0, 0.1, 10.0 };
  ofParameter<float> minWeightParameter { "minWeight", 10.0, 1.0, 50.0 };
  ofParameter<float> maxWeightParameter { "maxWeight", 50.0, 1.0, 100.0 };
  ofParameter<float> field1MultiplierParameter { "field1Multiplier", 1.0, 0.0, 2.0 };
  ofParameter<float> field2MultiplierParameter { "field2Multiplier", 1.0, 0.0, 2.0 };
  ofParameterGroup& getParameterGroup();

private:
  float getVelocityDampingEffective() const;
  float getForceMultiplierEffective() const;
  float getMaxVelocityEffective() const;
  float getParticleSizeEffective() const;
  float getJitterStrengthEffective() const;
  float getJitterSmoothingEffective() const;
  float getSpeedThresholdEffective() const;
  float getMinWeightEffective() const;
  float getMaxWeightEffective() const;
  float getField1MultiplierEffective() const;
  float getField2MultiplierEffective() const;

  void updateOverrides(const ParameterOverrides& overrides);

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

  ParameterOverrides parameterOverrides;

  ofVboMesh mesh;
  ofFloatColor particleColor;

  DrawShader drawShader;
  UpdateShader updateShader;
  InitShader initShader;

  float field1ValueOffset, field2ValueOffset; // -0.5 when values are [0,v]; 0.0 when values are [-v,v]
  ofTexture field1Texture, field2Texture;
  ofTexture emptyFieldTexture;

};



} // namespace ofxParticleField
