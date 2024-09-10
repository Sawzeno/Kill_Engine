#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;
layout(set = 1, binding = 0) uniform localUniformObject{
  vec4 diffuseColor;
}UBO;

layout(set = 1, binding = 1) uniform sampler2D diffuse_sampler;

layout(location = 1) in struct dto{
  vec2 tex_coord;
}in_dto;

void main(){

  // out_colour = vec4(in_position.r + 0.5, in_position.g + 0.5, in_position.b + 0.5, 1.0);
  // out_colour = vec4(1.0, 0.5, 0.0, 1.0);
  out_colour  = UBO.diffuseColor * texture(diffuse_sampler, in_dto.tex_coord);
}
