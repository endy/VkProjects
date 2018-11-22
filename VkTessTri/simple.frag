#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform blend_consts {
   vec4 scalar;
} blend_const;

flat layout(location = 0) in vec2 tesData;

void main() {
    outColor = vec4(tesData.x, 0.1, tesData.y, 1.0);
}
