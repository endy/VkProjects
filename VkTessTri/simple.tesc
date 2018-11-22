#version 450
#extension GL_ARB_separate_shader_objects : enable



layout(vertices = 3) out;

void main() {

    gl_TessLevelOuter[0] = 4.0f;
    gl_TessLevelOuter[1] = 4.0f;
    gl_TessLevelOuter[2] = 4.0f;
    gl_TessLevelOuter[3] = 4.0f;
    gl_TessLevelInner[0] = 4.0f;
    gl_TessLevelInner[1] = 4.0f;
}
