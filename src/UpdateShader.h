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
  void render(PingPongFbo& particleData) override {
    particleData.getTarget().begin();
    particleData.getTarget().activateAllDrawBuffers();
    shader.begin();
    shader.setUniformTexture("positionData", particleData.getSource().getTexture(POSITION_DATA_INDEX), 0);
    shader.setUniformTexture("velocityData", particleData.getSource().getTexture(VELOCITY_DATA_INDEX), 1);
    shader.setUniform1f("maxVelocity", 0.01f);
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
                uniform float maxVelocity;
                uniform sampler2DRect positionData;
                uniform sampler2DRect velocityData;
                layout(location = 0) out vec4 outPosition;
                layout(location = 1) out vec4 outVelocity;
                
                void main(void) {
                  vec2 position = texture(positionData, texCoordVarying).xy;
                  vec2 velocity = texture(velocityData, texCoordVarying).xy;
                  
                  outPosition = vec4(position + velocity*maxVelocity, 0.0, 1.0);
                  outVelocity = vec4(velocity, 0.0, 1.0);
                }
                );
  }
  
};



}
