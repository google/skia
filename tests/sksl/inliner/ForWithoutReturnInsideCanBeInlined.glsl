
out vec4 sk_FragColor;
uniform int value;
vec4 loopy(int v) {
    vec4 result = vec4(1.0);
    for (int x = 0;x < 5; ++x) {
        if (x == v) result = vec4(0.5);
    }
    return result;
}
void main() {
    sk_FragColor = loopy(value);
}
