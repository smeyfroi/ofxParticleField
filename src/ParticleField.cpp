#include "ParticleField.h"
#include <cmath>
#include "ofRandomEngine.h"
#include "ofLog.h"

namespace ofxParticleField {



ParticleField::ParticleField() {
  ofPixels emptyFieldPixels;
  emptyFieldPixels.allocate(1, 1, OF_PIXELS_RG);
  emptyFieldPixels.setColor(ofColor::black);
  ofDisableArbTex();
  emptyFieldTexture.allocate(emptyFieldPixels);
  emptyFieldTexture.loadData(emptyFieldPixels);
}

void ParticleField::setup(int approxNumParticles, ofFloatColor particleColor_, float field1ValueOffset_, float field2ValueOffset_) {
  particleColor = particleColor_;
  field1ValueOffset = field1ValueOffset_;
  field2ValueOffset = field2ValueOffset_;
  size_t particleDataW = (size_t)std::sqrt((float)approxNumParticles);
  size_t particleDataH = approxNumParticles / particleDataW;
  
  particleDataFbo.allocate(createParticleDataFboSettings(particleDataW, particleDataH));
  
  drawShader.load();
  updateShader.load();
  initShader.load();
  
  particleDataFbo.getSource().begin();
  for (size_t i = 0; i < numDataBuffers; ++i) {
    initShader.initializeRegion(particleDataFbo.getTarget(), i, 0, 0, particleDataW, particleDataH, ofRandom(10000.0f, 99999.0f) + i * 123.45f);
  }
  particleDataFbo.getSource().end();
  
  mesh.setMode(OF_PRIMITIVE_POINTS);
  for (size_t x = 0; x < particleDataW; ++x) {
    for (size_t y = 0; y < particleDataH; ++y) {
      mesh.addVertex({ (float)x, (float)y, 0.0f });
      mesh.addTexCoord({ (float)x, (float)y });
      mesh.addColor(particleColor);
    }
  }
}

void ParticleField::resizeParticles(int newApproxNumParticles) {
  size_t oldWidth = particleDataFbo.getWidth();
  size_t oldHeight = particleDataFbo.getHeight();
  size_t oldCount = oldWidth * oldHeight;
  
  size_t newWidth = (size_t)std::sqrt((float)newApproxNumParticles);
  size_t newHeight = newApproxNumParticles / newWidth;
  size_t newCount = newWidth * newHeight;
  
  if (oldWidth == newWidth && oldHeight == newHeight) {
    return;
  }
  
  ofFbo tempFbo;
  tempFbo.allocate(createParticleDataFboSettings(oldWidth, oldHeight));
  
  tempFbo.begin();
  for (size_t i = 0; i < numDataBuffers; ++i) {
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
    ofSetColor(255);
    particleDataFbo.getSource().getTexture(i).draw(0, 0);
  }
  tempFbo.end();
  
  particleDataFbo.allocate(createParticleDataFboSettings(newWidth, newHeight));
  
  size_t minWidth = (oldWidth < newWidth) ? oldWidth : newWidth;
  size_t minHeight = (oldHeight < newHeight) ? oldHeight : newHeight;
  
  particleDataFbo.getSource().begin();
  for (size_t i = 0; i < numDataBuffers; ++i) {
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    ofSetColor(255);
    tempFbo.getTexture(i).draw(0, 0, minWidth, minHeight);
  }
  
  if (newCount > oldCount) {
    size_t initMinHeight = (oldHeight < newHeight) ? oldHeight : newHeight;
    if (newWidth > oldWidth) {
      for (size_t i = 0; i < numDataBuffers; ++i) {
        initShader.initializeRegion(particleDataFbo.getTarget(), i, oldWidth, 0, newWidth - oldWidth, initMinHeight, ofRandom(10000.0f, 99999.0f) + i * 123.45f);
      }
    }
    if (newHeight > oldHeight) {
      for (size_t i = 0; i < numDataBuffers; ++i) {
        initShader.initializeRegion(particleDataFbo.getTarget(), i, 0, oldHeight, newWidth, newHeight - oldHeight, ofRandom(10000.0f, 99999.0f) + i * 123.45f);
      }
    }
  }
  particleDataFbo.getSource().end();
  
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_POINTS);
  for (size_t x = 0; x < newWidth; ++x) {
    for (size_t y = 0; y < newHeight; ++y) {
      mesh.addVertex({ (float)x, (float)y, 0.0f });
      mesh.addTexCoord({ (float)x, (float)y });
      mesh.addColor(particleColor);
    }
  }
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
                        maxVelocityParameter, jitterStrengthParameter, jitterSmoothingParameter);
  } else if (field1Texture.isAllocated()) {
    updateShader.render(particleDataFbo,
                        field1Texture, emptyFieldTexture,
                        field1ValueOffset, field2ValueOffset,
                        velocityDampingParameter, forceMultiplierParameter,
                        maxVelocityParameter, jitterStrengthParameter, jitterSmoothingParameter);
  }
}

void ParticleField::draw(ofFbo& foregroundFbo) {
  drawShader.render(mesh, foregroundFbo, particleDataFbo, particleSizeParameter, speedThresholdParameter);
}

ofParameterGroup& ParticleField::getParameterGroup() {
  if (parameters.size() == 0) {
    parameters.setName(getParameterGroupName());
    parameters.add(velocityDampingParameter);
    parameters.add(forceMultiplierParameter);
    parameters.add(maxVelocityParameter);
    parameters.add(particleSizeParameter);
    parameters.add(jitterStrengthParameter);
    parameters.add(jitterSmoothingParameter);
    parameters.add(speedThresholdParameter);
  }
  return parameters;
}



} // namespace ofxParticleField
