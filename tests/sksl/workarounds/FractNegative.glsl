#version 400
out vec4 sk_FragColor;
void main() {
    sk_FragColor.x = (0.5 - sign(-42.0) * (0.5 - fract(abs(-42.0))));
}
