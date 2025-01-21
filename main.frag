#version 450

layout(push_constant) uniform upc {
  float aspect;
};

layout(set = 0, binding = 0) uniform sampler2D atlas;

struct s_chr {
  vec2 pos;
  vec2 size;
  vec4 uv;
};
layout(set = 1, binding = 0) readonly buffer b_chrs { s_chr cs[]; };

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

void main() {
  vec2 p = pos;
  p.y /= aspect;
  p = p * 0.5 + 0.5;

  vec4 c = texture(atlas, p);
  colour = c;
}
