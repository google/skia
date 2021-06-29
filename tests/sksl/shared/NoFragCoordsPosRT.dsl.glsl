#version 400
out vec4 sk_FragCoord_Workaround;
layout (set = 0) uniform vec4 sk_RTAdjust;
layout (location = 0) in vec4 pos;
void main() {
    sk_FragCoord_Workaround = (gl_Position = pos);
}
