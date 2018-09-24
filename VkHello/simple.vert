#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 1) out vec2 texCoord;

vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0)
);

vec2 texcoords[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)

);

layout(location = 0) in vec2 inPosition;

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
        texCoord = texcoords[gl_VertexIndex] / 2.0;
    }
    else    
    {
        gl_Position = vec4(inPosition.xy, 0.5, 1.0);

        texCoord = texcoords[3*(gl_VertexIndex/3) + gl_VertexIndex%3] / 2.0;
    }
}
