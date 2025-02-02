#version 450

layout(push_constant) uniform upc {
  float aspect;
};
 
layout(set = 0, binding = 0) uniform sampler2D text;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 colour;

vec3 subtitle(vec2 p, vec3 c) {
  p = clamp(p * 0.5 + 0.5, 0, 1);

  float d = texture(text, p).r * 10;
  
  float dc = smoothstep(0, 3, d);
  float cc = cos(40 * dc) * 0.5 + 0.5;
  c = mix(vec3(cc * 0.3), c, clamp(dc * 3, 0, 1));
  c = mix(vec3(0), c, smoothstep(0.2, 0.3, d));
  c = mix(vec3(0.3, 0.5, 0.9), c, step(0.01, d));
  return c;
}

void main() {
  vec2 p = pos;
  p.y /= aspect;

  vec3 c = vec3(0.1);
  c = subtitle(p, c);

  colour = vec4(c, 1);
}
