
out vec4 sk_FragColor;
void main() {
    float x = -5.0;
    sk_FragColor.x = min(abs(x), 6.0);
}
