
out vec4 sk_FragColor;
vec4 adjust(vec4 v) {
    return v + vec4(0.125);
}
void main() {
    sk_FragColor = vec4(0.0);
    do sk_FragColor = adjust(sk_FragColor); while (sk_FragColor.x < 0.5);
}
