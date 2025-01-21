#version 450

layout(location = 0) in vec2 pos;

void main() {
  vec2 p = pos * 2.0 - 1.0;
  gl_Position = vec4(p, 0, 1);
}
