
out vec4 sk_FragColor;
vec4 adjust(vec4 v) {
    return v + vec4(0.125);
}
void main() {
    sk_FragColor = vec4(0.0);
    for (int x = 0;x < 4; ++x) sk_FragColor = adjust(sk_FragColor);
}
