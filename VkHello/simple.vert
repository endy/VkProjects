#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
    vec4 gl_Position;
};

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

layout(location = 0) in vec4 inPosition;

layout(push_constant) uniform vs_constants {
    float select;
    float unused0;
    float unused1;
    float unused2;
} constants;

void main() {
    if (constants.select > 0.1)
    {
        gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    }
    else    
    {
        gl_Position = inPosition;
    }
}
