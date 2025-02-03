#version 450

layout(set = 0, binding = 0) uniform sampler2D buf;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

void main() {
  vec2 p = clamp(pos * 0.5 + 0.5, 0, 1);
  colour = vec4(p, 0, 1);
}
