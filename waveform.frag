#version 450

layout(set = 0, binding = 0) uniform sampler2D buf;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

void main() {
  float c = texture(buf, vec2(pos.x, 0)).r;
  if (c > pos.y) c = 0;
  colour = vec4(0, c, 0, 1);
}
