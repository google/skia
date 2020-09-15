
out vec4 sk_FragColor;
uniform int value;
vec4 loopy(int v) {
    for (int x = 0;x < 5; ++x) {
        if (x == v) return vec4(0.5);
    }
    return vec4(1.0);
}
void main() {
    sk_FragColor = loopy(value);
}
