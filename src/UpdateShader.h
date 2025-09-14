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
  void render(PingPongFbo& particleData, ofTexture& fieldTexture, float velocityDamping, float forceMultiplier, float maxVelocity, float fieldValueOffset) {
    particleData.getTarget().begin();
    particleData.getTarget().activateAllDrawBuffers();
    shader.begin();
    shader.setUniformTexture("positionData", particleData.getSource().getTexture(POSITION_DATA_INDEX), 0);
    shader.setUniformTexture("velocityData", particleData.getSource().getTexture(VELOCITY_DATA_INDEX), 1);
    shader.setUniformTexture("fieldTexture", fieldTexture, 2); // FIXME: put `2` into Constants?
    shader.setUniform1f("velocityDamping", velocityDamping);
    shader.setUniform1f("forceMultiplier", forceMultiplier);
    shader.setUniform1f("maxVelocity", maxVelocity);
    shader.setUniform1f("fieldValueOffset", fieldValueOffset);
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
                uniform sampler2D fieldTexture;
                uniform float velocityDamping;
                uniform float forceMultiplier;
                uniform float maxVelocity;
                uniform float fieldValueOffset;
                layout(location = 0) out vec4 outPosition;
                layout(location = 1) out vec4 outVelocity;
                
                void main(void) {
                  vec2 normalizedParticlePosition = texture(positionData, texCoordVarying).xy;
                  vec2 velocity = texture(velocityData, texCoordVarying).xy;
                  vec2 field = texture(fieldTexture, normalizedParticlePosition).xy;

                  // FIXME: where are these NaNs coming from with the VideoFlowSourceMod?
                  if (isnan(field.x)) field.x = 0.0;
                  if (isnan(field.y)) field.y = 0.0;
                  
                  velocity = velocity + (field + fieldValueOffset) * forceMultiplier;
                  velocity *= velocityDamping;
                  
                  outPosition = vec4(fract(normalizedParticlePosition + velocity*maxVelocity), 0.0, 1.0);
                  outVelocity = vec4(velocity, 0.0, 1.0);
                }
                );
  }
  
};



}
