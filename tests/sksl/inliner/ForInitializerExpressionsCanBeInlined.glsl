
out vec4 sk_FragColor;
bool shouldLoop(vec4 v) {
    return v.x < 0.5;
}
vec4 grow(vec4 v) {
    return v + vec4(0.125);
}
void main() {
    for (sk_FragColor = vec4(0.0625);shouldLoop(sk_FragColor); sk_FragColor = grow(sk_FragColor)) {
    }
}
