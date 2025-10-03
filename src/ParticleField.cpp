#include "ParticleField.h"
#include <cmath>
#include "ofRandomEngine.h"
#include "ofLog.h"
#include "ofApp.h"
#include "Constants.h"

namespace ofxParticleField {



ParticleField::ParticleField() {
  ofPixels emptyFieldPixels;
  emptyFieldPixels.allocate(1, 1, OF_PIXELS_RG);
  emptyFieldPixels.setColor(ofColor::black);
  
  ofDisableArbTex();
  emptyFieldTexture.allocate(emptyFieldPixels);
  emptyFieldTexture.loadData(emptyFieldPixels);
}

void ParticleField::setup(int approxNumParticles, ofFloatColor particleColor, float field1ValueOffset_, float field2ValueOffset_) {
  field1ValueOffset = field1ValueOffset_;
  field2ValueOffset = field2ValueOffset_;
  size_t particleDataW = (size_t)std::sqrt((float)approxNumParticles);
  size_t particleDataH = approxNumParticles / particleDataW;
  
  particleDataFbo.allocate(createParticleDataFboSettings(particleDataW, particleDataH));
  setupParticlePositions();
  setupParticleVelocities();
  
  mesh.setMode(OF_PRIMITIVE_POINTS);
  for (size_t x = 0; x < particleDataW; ++x) {
    for (size_t y = 0; y < particleDataH; ++y) {
      mesh.addVertex({ (float)x, (float)y, 0.0f });
      mesh.addTexCoord({ (float)x, (float)y });
      mesh.addColor(particleColor);
    }
  }
  
  drawShader.load();
  updateShader.load();
}

ofFboSettings ParticleField::createParticleDataFboSettings(size_t width, size_t height) const {
  ofFboSettings fboSettings;
  fboSettings.width = width;
  fboSettings.height = height;
  fboSettings.numColorbuffers = numDataBuffers;
  fboSettings.internalformat = GL_RG32F; // full float precision to avoid accumulation artefacts
  fboSettings.textureTarget = GL_TEXTURE_RECTANGLE; // non-power-of-two texture with coordinates in pixel units
  fboSettings.minFilter = GL_NEAREST;
  fboSettings.maxFilter = GL_NEAREST;
  fboSettings.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
  fboSettings.wrapModeVertical = GL_CLAMP_TO_EDGE;
  return fboSettings;
}

void ParticleField::setupParticlePositions() {
  size_t width = particleDataFbo.getWidth();
  size_t height = particleDataFbo.getHeight();
  float* particlePositions = new float[width * height * 2];
  for (unsigned y = 0; y < height; ++y)
  {
    for (unsigned x = 0; x < width; ++x)
    {
      unsigned idx = y * width + x;
      particlePositions[idx * 2 + 0] = ofRandom(1.0) + 0.0;
      particlePositions[idx * 2 + 1] = ofRandom(1.0) + 0.0;
    }
  }
  loadParticleData(POSITION_DATA_INDEX, particlePositions);
  delete[] particlePositions;
}

void ParticleField::setupParticleVelocities() {
  size_t width = particleDataFbo.getWidth();
  size_t height = particleDataFbo.getHeight();
  float* particleVelocities = new float[width * height * 2];
  for (unsigned y = 0; y < height; ++y)
  {
    for (unsigned x = 0; x < width; ++x)
    {
      unsigned idx = y * width + x;
      particleVelocities[idx * 2 + 0] = ofRandom(-0.2, 0.2);
      particleVelocities[idx * 2 + 1] = ofRandom(-0.2, 0.2);
    }
  }
  loadParticleData(VELOCITY_DATA_INDEX, particleVelocities);
  delete[] particleVelocities;
}

void ParticleField::loadParticleData(size_t dataIndex, const float* data) {
  if (dataIndex > numDataBuffers) {
    ofLogError() << "ofxParticleField::loadParticleData: invalid data index " << dataIndex;
    return;
  }
  size_t width = particleDataFbo.getWidth();
  size_t height = particleDataFbo.getHeight();
  particleDataFbo.getSource().getTexture(dataIndex).loadData(data, width, height, GL_RGB);
}

void ParticleField::setField1(const ofTexture& fieldTexture) {
  field1Texture = fieldTexture; // shares GPU texture with the owner
}

void ParticleField::setField2(const ofTexture& fieldTexture) {
  field2Texture = fieldTexture; // shares GPU texture with the owner
}

void ParticleField::update() {
  if (field2Texture.isAllocated() && field1Texture.isAllocated()) {
    updateShader.render(particleDataFbo,
                        field1Texture, field2Texture,
                        field1ValueOffset, field2ValueOffset,
                        velocityDampingParameter, forceMultiplierParameter,
                        maxVelocityParameter, jitterStrengthParameter);
  } else if (field1Texture.isAllocated()) {
    updateShader.render(particleDataFbo,
                        field1Texture, emptyFieldTexture,
                        field1ValueOffset, field2ValueOffset,
                        velocityDampingParameter, forceMultiplierParameter,
                        maxVelocityParameter, jitterStrengthParameter);
  }
}

void ParticleField::draw(ofFbo& foregroundFbo) {
  drawShader.render(mesh, foregroundFbo, particleDataFbo, particleSizeParameter, speedThresholdParameter);

//  foregroundFbo.begin();
//  ofPushStyle();
//  ofEnableBlendMode(OF_BLENDMODE_ALPHA);
//  ofSetColor(ofFloatColor { 1.0, 1.0, 1.0, 0.5 });
//  field1Texture.draw(0.0, 0.0, foregroundFbo.getWidth(), foregroundFbo.getHeight());
//  field2Texture.draw(0.0, 0.0, foregroundFbo.getWidth(), foregroundFbo.getHeight());
//  ofPopStyle();
//  foregroundFbo.end();
}

ofParameterGroup& ParticleField::getParameterGroup() {
  if (parameters.size() == 0) {
    parameters.setName(getParameterGroupName());
    parameters.add(velocityDampingParameter);
    parameters.add(forceMultiplierParameter);
    parameters.add(maxVelocityParameter);
    parameters.add(particleSizeParameter);
    parameters.add(jitterStrengthParameter);
    parameters.add(speedThresholdParameter);
  }
  return parameters;
}



} // namespace ofxParticleField
