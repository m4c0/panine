#version 450
#extension GL_GOOGLE_include_directive : require
#include "../glslinc/inigo.glsl"

layout(push_constant) uniform upc {
  float aspect;
};
 
layout(set = 0, binding = 0) uniform sampler2D text;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

void main() {
  vec2 p = pos;
  p.y /= aspect;
  p = clamp(p * 0.5 + 0.5, 0, 1);

  float d = texture(text, p).r;
  
  vec3 c = inigo_debug(d);

  colour = vec4(c, 1);
}
