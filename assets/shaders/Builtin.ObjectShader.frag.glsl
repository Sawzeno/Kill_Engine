#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;

layout(location = 0) out vec4 out_colour;

vec2 offset = {0.5,0.0};
void main(){
  out_colour = vec4(in_position.r + offset.x,
                    in_position.g + offset.x,
                    in_position.b + offset.x,
                    1.0);
}
