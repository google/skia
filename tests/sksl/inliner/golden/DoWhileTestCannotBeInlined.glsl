
out vec4 sk_FragColor;
bool shouldLoop(vec4 v) {
    return v.x < 0.5;
}
void main() {
    sk_FragColor = vec4(0.0);
    do {
        sk_FragColor += vec4(0.125);
    } while (shouldLoop(sk_FragColor));
}
