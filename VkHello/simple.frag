#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform blend_consts {
   vec4 scalar;
} blend_const;

void main() {
    outColor = vec4(0.7, 0.3, 0.7, 1.0) * blend_const.scalar;
}
