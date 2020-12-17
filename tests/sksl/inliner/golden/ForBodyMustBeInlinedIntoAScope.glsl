
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(0.0);
    for (int x = 0;x < 4; ++x) {
        sk_FragColor = sk_FragColor + vec4(0.125);
    }
}
