//
//  UpdateShader.h
//  example_noise
//
//  Created by Steve Meyfroidt on 07/09/2025.
//

#pragma once

#include "Shader.h"
#include "Constants.h"

namespace ofxParticleField {



class UpdateShader : public Shader {
  
public:
  void render(PingPongFbo& particleData, const ofTexture& field1Texture, const ofTexture& field2Texture, float field1ValueOffset, float field2ValueOffset, float velocityDamping, float forceMultiplier, float maxVelocity, float jitterStrength) {
    particleData.getTarget().begin();
    particleData.getTarget().activateAllDrawBuffers();
    shader.begin();
    shader.setUniformTexture("positionData", particleData.getSource().getTexture(POSITION_DATA_INDEX), 0);
    shader.setUniformTexture("velocityData", particleData.getSource().getTexture(VELOCITY_DATA_INDEX), 1);
    shader.setUniformTexture("jitterData", particleData.getSource().getTexture(JITTER_DATA_INDEX), 2);
    shader.setUniformTexture("field1Texture", field1Texture, 3);
    shader.setUniformTexture("field2Texture", field2Texture, 4);
    shader.setUniform1f("field1ValueOffset", field1ValueOffset);
    shader.setUniform1f("field2ValueOffset", field2ValueOffset);
    shader.setUniform1f("velocityDamping", velocityDamping);
    shader.setUniform1f("forceMultiplier", forceMultiplier);
    shader.setUniform1f("maxVelocity", maxVelocity);
    shader.setUniform1f("jitterStrength", jitterStrength);
    shader.setUniform1f("jitterSeed", ofGetElapsedTimef());
    particleData.getSource().draw(0, 0);
    shader.end();
    particleData.getTarget().end();
    particleData.swap();
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
                uniform sampler2DRect positionData;
                uniform sampler2DRect velocityData;
                uniform sampler2DRect jitterData;
                uniform sampler2D field1Texture;
                uniform sampler2D field2Texture;
                uniform float field1ValueOffset;
                uniform float field2ValueOffset;
                uniform float velocityDamping;
                uniform float forceMultiplier;
                uniform float maxVelocity;
                uniform float jitterStrength;
                uniform float jitterSeed;
                layout(location = 0) out vec4 outPosition;
                layout(location = 1) out vec4 outVelocity;
                layout(location = 2) out vec4 outJitter;
                
                // Cheap per-pixel RNG (Interleaved Gradient Noise)
                float ign(vec2 p) {
                  return fract(52.9829189 * fract(0.06711056 * p.x + 0.00583715 * p.y));
                }

                void main(void) {
                  vec2 normalizedParticlePosition = texture(positionData, texCoordVarying).xy;
                  vec2 velocity = texture(velocityData, texCoordVarying).xy;
                  vec2 jitterSmooth = texture(jitterData, texCoordVarying).xy;
                  vec2 field1 = texture(field1Texture, normalizedParticlePosition).xy + field1ValueOffset;
                  vec2 field2 = texture(field2Texture, normalizedParticlePosition).xy + field2ValueOffset;
                  vec2 field = field1 + field2;

                  // FIXME: where are these NaNs coming from with the VideoFlowSourceMod?
                  if (isnan(field.x)) field.x = 0.0;
                  if (isnan(field.y)) field.y = 0.0;
                  
                  vec2 rnd = vec2(
                    ign(gl_FragCoord.xy + vec2(jitterSeed, jitterSeed * 1.37)),
                    ign(gl_FragCoord.yx + vec2(jitterSeed * 2.17, jitterSeed * 3.13))
                  );
                  vec2 jitterRaw = (rnd - 0.5) * (2.0 * jitterStrength);
                  
                  float jitterSmoothing = 0.1;
                  jitterSmooth = mix(jitterSmooth, jitterRaw, jitterSmoothing);

                  velocity += field * forceMultiplier;
                  velocity += jitterSmooth;
                  velocity *= velocityDamping;
                  
                  outPosition = vec4(fract(normalizedParticlePosition + velocity*maxVelocity), 0.0, 1.0);
                  outVelocity = vec4(velocity, 0.0, 1.0);
                  outJitter = vec4(jitterSmooth, 0.0, 1.0);
                }
                );
  }
  
};



}
