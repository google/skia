#version 400
out vec4 sk_FragCoord_Workaround;
layout (location = 0) in vec4 pos;
void main() {
    sk_FragCoord_Workaround = (gl_Position = pos);
}
