#version 110
#extension GL_ARB_fragment_coord_conventions : require
layout(origin_upper_left) in vec4 gl_FragCoord;
void main() {
    gl_FragColor.xy = gl_FragCoord.xy;
}
