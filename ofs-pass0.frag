#version 450

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
  p = clamp(p * 0.5 + 0.5, 0, 1);

  float c = 0;

  vec2 pp = p * 1024.0;
  for (int i = 0; i < 16; i++) {
    vec2 pc = (pp - cs[i].pos) / cs[i].size;
    vec2 dc = abs(pc - 0.5) - 0.5;
    float dd = 1.0 - step(0, max(dc.x, dc.y));

    vec2 n = clamp(pc, 0, 1);
    vec2 uv = cs[i].uv.xy + n * cs[i].uv.zw;
    vec4 cc = texture(atlas, uv);
    c = mix(c, 1, cc.r * dd);
  }

  c = 1 - c;
  colour = vec4(c, 0, 0, 1);
}
