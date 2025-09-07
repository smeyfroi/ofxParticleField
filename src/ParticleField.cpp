#include "ParticleField.h"
#include <cmath>
#include "ofRandomEngine.h"
#include "ofLog.h"
#include "ofApp.h"
#include "Constants.h"

namespace ofxParticleField {



void ParticleField::setup(int approxNumParticles) {
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
  fboSettings.internalformat = GL_RGB16F; // vec3 half-float precision for particle data
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
  float* particlePositions = new float[width * height * 4];
  for (unsigned y = 0; y < height; ++y)
  {
    for (unsigned x = 0; x < width; ++x)
    {
      unsigned idx = y * width + x;
      particlePositions[idx * 4 + 0] = ofRandom(1.0);
      particlePositions[idx * 4 + 1] = ofRandom(1.0);
      particlePositions[idx * 4 + 2] = 0.0;
    }
  }
  loadParticleData(POSITION_DATA_INDEX, particlePositions);
  delete[] particlePositions;
}

void ParticleField::setupParticleVelocities() {
  size_t width = particleDataFbo.getWidth();
  size_t height = particleDataFbo.getHeight();
  float* particleVelocities = new float[width * height * 4];
  for (unsigned y = 0; y < height; ++y)
  {
    for (unsigned x = 0; x < width; ++x)
    {
      unsigned idx = y * width + x;
      particleVelocities[idx * 4 + 0] = ofRandom(-1.0, 1.0);
      particleVelocities[idx * 4 + 1] = ofRandom(-1.0, 1.0);
      particleVelocities[idx * 4 + 2] = 0.0;
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

void ParticleField::setFieldTexture(ofTexture&& tex) { // FIXME: should this be a shared_ptr?
  fieldTexture = std::move(tex);
}

void ParticleField::update() {
  updateShader.render(particleDataFbo);
}

void ParticleField::draw(ofFbo& foregroundFbo) {
  drawShader.render(mesh, foregroundFbo, particleDataFbo);
}



}
