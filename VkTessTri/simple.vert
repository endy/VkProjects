#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
    vec4 gl_Position;
};

// layout(location = 1) out vec2 texCoord;

vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0)
);

layout(push_constant) uniform vs_constants {
    float select;
    float unused0;
    float unused1;
    float unused2;
} constants;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
