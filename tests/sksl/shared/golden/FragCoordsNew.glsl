#version 400
layout(origin_upper_left) in vec4 gl_FragCoord;
out vec4 sk_FragColor;
void main() {
    sk_FragColor.xy = gl_FragCoord.xy;
}
