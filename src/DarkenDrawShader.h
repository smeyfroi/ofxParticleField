//
//  DarkenDrawShader.h
//  ofxParticleField
//
//  Halo-free particle blend: instead of fixed-function alpha blending
//  (which causes a brightness halo at the edges of dark particle deposits
//  on translucent destinations because dst.alpha grows faster than dst.rgb
//  darkens), this shader reads a SNAPSHOT of the destination FBO and
//  computes the new pixel value as a modulate-darken with colour tint.
//  Critically, dst.alpha is PRESERVED in the output — only rgb changes.
//  No alpha change → no halo from alpha-vs-rgb mismatch at display composite.
//
//  Per-channel formula:
//    perChannelFactor = 1 - darknessAmount * (1 - srcColour.rgb)
//    new.rgb = scratch.rgb * perChannelFactor
//    new.a   = scratch.a   (UNCHANGED)
//
//  Semantics by srcColour:
//    Pure BLACK  (rgb=0):  uniform darken by darknessAmount across channels
//    Pure WHITE  (rgb=1):  perChannelFactor = 1 — no effect (white can't darken)
//    Coloured src (e.g. red (1,0,0)): darkens the NON-tint channels (g,b)
//      while preserving the tint channel — net effect is a "shade toward
//      this colour" by removing the complementary channels.
//
//  Caller responsibilities:
//    - The destination FBO must already contain a SNAPSHOT (the source-to-
//      scratch copy is done by `drawDarken` in ParticleField; this shader
//      just reads from the bound scratch texture).
//    - GL blend should be DISABLED during the draw — the shader writes the
//      final composed pixel value, not a delta to be blended.
//    - Point sprites with discard outside circle, alpha-from-motion as in
//      DrawShader.
//
//  Multi-particle overlap: with blend disabled, overlapping particles in the
//  same draw call only the last-drawn fragment "wins" at each pixel (since
//  all particles read the SAME scratch snapshot). For sparse particle counts
//  (~100s per frame) collisions are rare. If collisions become visible, the
//  fix is sequential draws or framebuffer-fetch (Apple-only extension).
//
//  Performance: each drawDarken() call requires one full-FBO copy of the
//  destination into the lazy-allocated scratch FBO, plus one extra FBO of
//  destination size held in memory persistently. At 4800² RGBA32F that's
//  ~92 MB/frame copy + 92 MB resident per ParticleField mod using DARKEN
//  mode. Bandwidth at 60 fps is roughly ~5.5 GB/s — well within modern
//  desktop GPU capacity but enough to warn against blanket use across
//  every ParticleField in a grid. Opt-in per cell, used when the slab-
//  aesthetic or ink-on-paint visual identity requires it.

#pragma once

#include "Shader.h"
#include "Constants.h"

namespace ofxParticleField {

class DarkenDrawShader : public Shader {

public:
  // scratchTex must contain a COPY of the FBO contents at draw start (so each
  // fragment reads pre-draw dst).
  void render(const ofVboMesh& mesh,
              const ofFbo& fbo,
              PingPongFbo& particleData,
              const ofTexture& scratchTex,
              float pointSize,
              float motionSensitivity) {
    ofPushStyle();
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Critical: blend OFF — shader writes final pixel, no blend.
    glDisable(GL_BLEND);

    fbo.begin();
    shader.begin();
    shader.setUniformTexture("positionData",
                             particleData.getSource().getTexture(POSITION_DATA_INDEX),
                             0);
    shader.setUniformTexture("velocityData",
                             particleData.getSource().getTexture(VELOCITY_DATA_INDEX),
                             1);
    shader.setUniformTexture("scratchTex", scratchTex, 2);
    shader.setUniform1i("renderW", fbo.getWidth());
    shader.setUniform1i("renderH", fbo.getHeight());
    shader.setUniform2f("fboSize", static_cast<float>(fbo.getWidth()),
                                   static_cast<float>(fbo.getHeight()));
    shader.setUniform1f("pointSize", pointSize);
    shader.setUniform1f("motionSensitivity", motionSensitivity);
    mesh.draw();
    shader.end();
    fbo.end();

    glDisable(GL_PROGRAM_POINT_SIZE);
    ofPopStyle();
  }

protected:
  std::string getVertexShader() override {
    // Same vertex stage as DrawShader: point sprite at particle position.
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
                uniform sampler2D scratchTex;
                uniform vec2 fboSize;
                uniform float motionSensitivity;
                out vec4 fragColor;

                void main(void) {
                  // Discard pixels outside the point sprite's circle.
                  vec2 cxy = 2.0 * gl_PointCoord - 1.0;
                  float r = dot(cxy, cxy);
                  if (r > 1.0) {
                    discard;
                  }

                  // Motion-based alpha falloff (same as DrawShader).
                  vec4 particleVelocity = texture(velocityData, texCoordVarying);
                  float speed = length(particleVelocity.xy);
                  speed = smoothstep(0.0, 1.0, speed * motionSensitivity);
                  float alphaFactor = clamp(speed - r, 0.0, 1.0);
                  float darknessAmount = clamp(colorVarying.a, 0.0, 1.0) * alphaFactor;

                  // Sample the destination snapshot at this pixel.
                  // gl_FragCoord origin is bottom-left; sampler2D normalised
                  // coords match if the scratch FBO was drawn-into without
                  // Y-flip (which is the OF default).
                  vec2 normCoord = gl_FragCoord.xy / fboSize;
                  vec4 dst = texture(scratchTex, normCoord);

                  // Per-channel modulate darken with colour tint.
                  // Black src (rgb=0) → factor = 1 - darknessAmount uniformly.
                  // White src (rgb=1) → factor = 1 (no change).
                  // Coloured src → preserves tint channels, darkens others.
                  vec3 perChannelFactor = vec3(1.0) - darknessAmount * (vec3(1.0) - colorVarying.rgb);
                  vec3 newRgb = dst.rgb * perChannelFactor;

                  // Alpha PRESERVED — this is what eliminates the halo.
                  fragColor = vec4(newRgb, dst.a);
                }
                );
  }
};

} // namespace ofxParticleField
