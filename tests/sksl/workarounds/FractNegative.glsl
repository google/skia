#version 400
out vec4 sk_FragColor;
void main() {
    float x = -42.0;
    sk_FragColor.x = (0.5 - sign(x) * (0.5 - fract(abs(x))));
}
