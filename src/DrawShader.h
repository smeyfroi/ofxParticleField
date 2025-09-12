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
  void render(const ofVboMesh& mesh, const ofFbo& fbo, PingPongFbo& particleData, float pointSize) {
    fbo.begin();
    ofClear(0, 0, 0, 0);
    shader.begin();
    shader.setUniformTexture("positionData", particleData.getSource().getTexture(POSITION_DATA_INDEX), 0);
    shader.setUniform1i("renderW", fbo.getWidth());
    shader.setUniform1i("renderH", fbo.getHeight());
    shader.setUniform1f("pointSize", pointSize);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    mesh.draw();
    shader.end();
    fbo.end();
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
                out vec4 fragColor;
                
                void main(void) {
                  // discard pixels outside circle
                  vec2 cxy = 2.0 * gl_PointCoord - 1.0;
                  float r = dot(cxy, cxy);
                  if (r > 1.0) {
                    discard;
                  }
                  // set alpha according to distance to center
                  float alpha = 1.0 - r;
                  fragColor = colorVarying;
                  fragColor.a *= alpha;
                }
                );
  }
  
};



}
