#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;

layout(set = 1, binding = 0) uniform localUniformObject{
  vec4 diffuse_colour;
}object_ubo;

//samplers
layout(set = 1, binding = 1) uniform sampler2D diffuse_sampler;

//data ttransfer obeject
layout(location = 1) in struct dto{
  vec2 tex_coord;
}in_dto;

void main(){
  out_colour = object_ubo.diffuse_colour;
}
