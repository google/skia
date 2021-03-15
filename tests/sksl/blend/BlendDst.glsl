#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
void main() {
    sk_FragColor = dst;
}
