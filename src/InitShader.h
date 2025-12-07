#pragma once

#include "Shader.h"

namespace ofxParticleField {

class InitShader : public Shader {
  
public:
  void initializeRegion(ofFbo& targetFbo, size_t dataIndex, size_t startX, size_t startY, size_t width, size_t height, float randomSeed, float minWeight = 0.5f, float maxWeight = 2.0f) {
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + dataIndex);
    
    ofPushView();
    ofViewport(0, 0, targetFbo.getWidth(), targetFbo.getHeight());
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, targetFbo.getWidth(), 0, targetFbo.getHeight(), -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    ofPushStyle();
    ofSetColor(255);
    ofFill();
    shader.begin();
    shader.setUniform1i("dataIndex", (int)dataIndex);
    shader.setUniform2f("fboSize", (float)targetFbo.getWidth(), (float)targetFbo.getHeight());
    shader.setUniform2f("regionStart", (float)startX, (float)startY);
    shader.setUniform1f("randomSeed", randomSeed);
    shader.setUniform1f("minWeight", minWeight);
    shader.setUniform1f("maxWeight", maxWeight);
    ofDrawRectangle(startX, startY, width, height);
    shader.end();
    ofPopStyle();
    
    glDrawBuffer(GL_COLOR_ATTACHMENT0); // Reset to default attachment
    ofPopView();
  }
  
protected:
  std::string getVertexShader() override {
    return GLSL(
                uniform mat4 modelViewProjectionMatrix;
                in vec4 position;
                in vec2 texcoord;
                out vec2 texCoordVarying;

                void main() {
                  gl_Position = modelViewProjectionMatrix * position;
                  texCoordVarying = texcoord;
                }
                );
  }
  
  std::string getFragmentShader() override {
    return GLSL(
                in vec2 texCoordVarying;
                uniform int dataIndex;
                uniform vec2 fboSize;
                uniform vec2 regionStart;
                uniform float randomSeed;
                uniform float minWeight;
                uniform float maxWeight;
                out vec4 fragColor;
                
                uint hash(uint x) {
                  x += (x << 10u);
                  x ^= (x >> 6u);
                  x += (x << 3u);
                  x ^= (x >> 11u);
                  x += (x << 15u);
                  return x;
                }
                
                uint hash(uvec2 v, uint seed) {
                  return hash(v.x ^ hash(v.y) ^ seed);
                }
                
                float floatConstruct(uint m) {
                  const uint ieeeMantissa = 0x007FFFFFu;
                  const uint ieeeOne = 0x3F800000u;
                  m &= ieeeMantissa;
                  m |= ieeeOne;
                  float f = uintBitsToFloat(m);
                  return f - 1.0;
                }
                
                float random(vec2 v, float seed) {
                  return floatConstruct(hash(uvec2(v), uint(seed * 1000.0)));
                }
                
                vec2 random2(vec2 p, float seed) {
                  return vec2(
                    random(p, seed),
                    random(p, seed + 1.0)
                  );
                }

                void main(void) {
                  vec2 pixelCoord = gl_FragCoord.xy;
                  vec2 rnd = random2(pixelCoord, randomSeed);
                  
                  if (dataIndex == 0) {
                    fragColor = vec4(rnd.x, rnd.y, 0.0, 1.0);
                  } else if (dataIndex == 1) {
                    vec2 velocity = (random2(pixelCoord, randomSeed + 123.456) - 0.5) * 0.4;
                    fragColor = vec4(velocity, 0.0, 1.0);
                  } else if (dataIndex == 3) {
                    float weight = mix(minWeight, maxWeight, random(pixelCoord, randomSeed + 789.123));
                    fragColor = vec4(weight, 0.0, 0.0, 1.0);
                  } else {
                    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
                  }
                }
                );
  }
  
};

}
