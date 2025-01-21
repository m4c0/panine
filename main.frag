#version 450

layout(set = 0, binding = 0) uniform sampler2D atlas;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

void main() {
  vec4 c = texture(atlas, pos);
  colour = c;
}
