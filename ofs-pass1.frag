#version 450

layout(set = 0, binding = 0) uniform sampler2D atlas;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

void main() {
  vec2 p = pos;
  p = clamp(p * 0.5 + 0.5, 0, 1);

  vec4 cc = texture(atlas, p);
  colour = vec4(cc.r);
}
