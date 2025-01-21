#version 450

layout(push_constant) uniform upc {
  float aspect;
};

layout(set = 0, binding = 0) uniform sampler2D atlas;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

void main() {
  vec2 p = pos;
  p.y /= aspect;
  p = p * 0.5 + 0.5;

  vec4 c = texture(atlas, p);
  colour = c;
}
