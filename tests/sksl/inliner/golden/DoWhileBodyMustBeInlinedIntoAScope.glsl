
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(0.0);
    do {
        sk_FragColor = sk_FragColor + vec4(0.125);
    } while (sk_FragColor.x < 0.5);
}
