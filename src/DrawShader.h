//
//  DrawShader.h
//  example_noise
//
//  Created by Steve Meyfroidt on 07/09/2025.
//

#pragma once

#include "Shader.h"
#include "Constants.h"

namespace ofxParticleField {



class DrawShader : public Shader {
  
public:
  void render(const ofVboMesh& mesh, const ofFbo& fbo, PingPongFbo& particleData, float pointSize, float speedThreshold) {
    ofPushStyle();
    glEnable(GL_PROGRAM_POINT_SIZE);
    fbo.begin();
    shader.begin();
    shader.setUniformTexture("positionData", particleData.getSource().getTexture(POSITION_DATA_INDEX), 0);
    shader.setUniformTexture("velocityData", particleData.getSource().getTexture(VELOCITY_DATA_INDEX), 1);
    shader.setUniform1i("renderW", fbo.getWidth());
    shader.setUniform1i("renderH", fbo.getHeight());
    shader.setUniform1f("pointSize", pointSize);
    shader.setUniform1f("speedThreshold", speedThreshold);
    mesh.draw();
    shader.end();
    fbo.end();
    glDisable(GL_PROGRAM_POINT_SIZE);
    ofPopStyle();
  }
  
protected:
  std::string getVertexShader() override {
    return GLSL(
                uniform mat4 modelViewProjectionMatrix;
                in vec2 texcoord;
                in vec4 color;
                uniform sampler2DRect positionData;
                uniform int renderW;
                uniform int renderH;
                uniform float pointSize;
                out vec2 texCoordVarying;
                out vec4 colorVarying;
                
                void main() {
                  vec4 normalizedParticlePosition = texture(positionData, texcoord);
                  vec4 position = vec4(normalizedParticlePosition.x * renderW,
                                       normalizedParticlePosition.y * renderH,
                                       0.0, 1.0);
                  gl_Position = modelViewProjectionMatrix * position;
                  gl_PointSize = pointSize;
                  texCoordVarying = texcoord;
                  colorVarying = color;
                }
                );
  }
  
  std::string getFragmentShader() override {
    return GLSL(
                in vec2 texCoordVarying;
                in vec4 colorVarying;
                uniform sampler2DRect velocityData;
                uniform float speedThreshold;
                out vec4 fragColor;
                
                void main(void) {
                  // discard pixels outside circle
                  vec2 cxy = 2.0 * gl_PointCoord - 1.0;
                  float r = dot(cxy, cxy);
                  if (r > 1.0) {
                    discard;
                  }
                  
                  vec4 particleVelocity = texture(velocityData, texCoordVarying);
                  float speed = length(particleVelocity.xy);
                  speed = smoothstep(0.0, 1.0, speed * speedThreshold);

                  // set alpha according to distance to center
                  float alpha = clamp(speed - r, 0.0, 1.0);

                  // Premultiplied alpha output.
                  float a = clamp(colorVarying.a, 0.0, 1.0) * alpha;
                  fragColor = vec4(colorVarying.rgb * a, a);
                }
                );
  }
  
};



}
