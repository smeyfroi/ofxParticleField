#pragma once

#include <functional>
#include <optional>

#include "DarkenDrawShader.h"
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
    std::optional<float> motionSensitivity;
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
  // Halo-free "modulate darken" draw. Reads a snapshot of foregroundFbo into
  // a private scratchFbo, then runs DarkenDrawShader which writes new pixel
  // values (rgb modulated, alpha PRESERVED). Use for slab-disruption /
  // ink-on-paint cells where standard premultiplied OVER would produce
  // brightness halos around dark deposits on translucent destinations.
  // See DarkenDrawShader.h for the per-channel math + tint semantics.
  void drawDarken(ofFbo& foregroundFbo, bool smallParticles = false);
  void setField1(const ofTexture& fieldTexture);
  void setField2(const ofTexture& fieldTexture);
  void updateRandomColorBlocks(int numBlocks, int blockSize, std::function<ofFloatColor(size_t)> colorFunc);

  int getParticleCount() const { return particleDataFbo.getWidth() * particleDataFbo.getHeight(); }

  std::string getParameterGroupName() const { return "Particle Field"; }
  ofParameterGroup parameters;
  ofParameter<float> ln2ParticleCountParameter { "ln2ParticleCount", 14.0, 4.0, 18.0 }; // 2^4 = 16, 2^18 = 262K
  ofParameter<float> velocityDampingParameter { "velocityDamping", 0.997, 0.99, 1.0 };
  ofParameter<float> forceMultiplierParameter { "forceMultiplier", 1.0, 0.0, 2.0 };
  ofParameter<float> maxVelocityParameter { "maxVelocity", 0.001, 0.0, 0.003 };
  ofParameter<float> particleSizeParameter { "particleSize", 8.0, 1.0, 16.0 };
  ofParameter<float> jitterStrengthParameter { "jitterStrength", 0.01, 0.0, 0.05 };
  ofParameter<float> jitterSmoothingParameter { "jitterSmoothing", 0.1, 0.0, 1.0 };
  ofParameter<float> motionSensitivityParameter { "motionSensitivity", 2.0, 0.1, 10.0 };
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
  float getMotionSensitivityEffective() const;
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
  DarkenDrawShader darkenDrawShader;
  UpdateShader updateShader;
  InitShader initShader;

  // Scratch FBO for drawDarken: holds a per-frame snapshot of the destination
  // FBO so the darken shader can read pre-draw dst values while writing new
  // ones to the destination itself (avoids read-write conflict on the same
  // texture). Lazy-allocated and re-sized to match the foreground FBO.
  ofFbo darkenScratchFbo;

  float field1ValueOffset, field2ValueOffset; // -0.5 when values are [0,v]; 0.0 when values are [-v,v]
  ofTexture field1Texture, field2Texture;
  ofTexture emptyFieldTexture;

};



} // namespace ofxParticleField
