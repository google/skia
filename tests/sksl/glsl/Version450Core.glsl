#version 450 core
out vec4 sk_FragColor;
in float test;
void main() {
    sk_FragColor.x = test;
}
