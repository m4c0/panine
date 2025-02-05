#version 450

layout(push_constant) uniform upc { float probe; };

layout(set = 0, binding = 0) uniform sampler2D buf;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

void main() {
  float c = texture(buf, vec2(pos.x, 0)).r;
  colour = vec4(probe, c, 0, 1);
}
