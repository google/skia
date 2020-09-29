
out vec4 sk_FragColor;
void main() {
    sk_FragColor.x = min(abs(-5.0), 6.0);
}
