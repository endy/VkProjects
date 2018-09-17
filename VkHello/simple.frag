#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform blend_consts {
   vec4 scalar;
} blend_const;

layout(set=0, binding=0) uniform texture2D tex;
layout(set=0, binding=1) uniform sampler samp;

layout (location = 1) in vec2 texCoord;

void main() {
    outColor = texture(sampler2D(tex,samp), texCoord); // * blend_const.scalar;
}
