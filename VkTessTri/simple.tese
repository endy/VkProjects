#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(triangles) in;
layout(location = 0) out vec2 tesData;

void main() {
    gl_Position = vec4(gl_TessCoord.xy, 0.0, 1.0);

    tesData.xy = gl_TessCoord.xy;
}
