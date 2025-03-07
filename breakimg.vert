#version 450

layout(push_constant) uniform upc {
  vec2 scale;
};

layout(location = 0) in vec2 pos;
layout(location = 0) out vec2 f_uv;

void main() {
  vec2 p = pos * 2.0 - 1.0;
  gl_Position = vec4(p / scale, 0, 1);
  f_uv = pos;
}

