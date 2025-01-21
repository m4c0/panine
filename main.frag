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
  p = clamp(p * 0.5 + 0.5, 0, 1);

  vec4 c = vec4(0, 0, 0, 1);

  vec2 pp = p * 1024.0;
  for (int i = 0; i < 5; i++) {
    vec2 n = clamp((pp - cs[i].pos) / cs[i].size, 0, 1);
    vec2 uv = cs[i].uv.xy + n * cs[i].uv.zw;
    vec4 cc = texture(atlas, uv);
    c = mix(c, cc, cc.a);
  }

  colour = c;
}
