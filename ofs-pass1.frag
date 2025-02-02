#version 450

layout(push_constant) uniform pc { vec2 ext; };
layout(set = 0, binding = 0) uniform sampler2D atlas;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

const float d = 1.0 / 256.0;
const float dd = 1.4 / 256.0;

void main() {
  vec2 p = clamp(pos * 0.5 + 0.5, 0, 1);
  vec2 e = 1.0 / ext;

  vec4 cc = texture(atlas, p);
  cc = min(cc, texture(atlas, p + vec2(e.x, 0)) + d);
  cc = min(cc, texture(atlas, p - vec2(e.x, 0)) + d);
  cc = min(cc, texture(atlas, p + vec2(0, e.y)) + d);
  cc = min(cc, texture(atlas, p - vec2(0, e.y)) + d);
  cc = min(cc, texture(atlas, p + vec2( e.x,  e.y)) + dd);
  cc = min(cc, texture(atlas, p + vec2(-e.x,  e.y)) + dd);
  cc = min(cc, texture(atlas, p + vec2( e.x, -e.y)) + dd);
  cc = min(cc, texture(atlas, p + vec2(-e.x, -e.y)) + dd);
  
  colour = vec4(cc.r);
}
