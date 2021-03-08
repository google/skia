
out vec4 sk_FragColor;
void main() {
    float x = -42.0;
    sk_FragColor.x = fract(x);
}
