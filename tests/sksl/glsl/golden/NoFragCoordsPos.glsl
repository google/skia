#version 400
out vec4 sk_FragCoord_Workaround;
in vec4 pos;
void main() {
    sk_FragCoord_Workaround = (gl_Position = pos);
}
