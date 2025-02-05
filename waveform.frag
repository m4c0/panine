#version 450

layout(push_constant) uniform upc { float probe; };

layout(set = 0, binding = 0) uniform sampler2D buf;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

void main() {
  float d = probe - pos.x;
  d = abs(d);

  float r = 1.0 - smoothstep(0, 0.01, d);
  float g = texture(buf, vec2(pos.x, 0)).r;
  colour = vec4(r, g, 0, 1);
}
