
out vec4 sk_FragColor;
void foo(out float x) {
    x = 42.0;
}
float bar(float y) {
    foo(y);
    return y + 1.0;
}
void main() {
    float z = bar(123.0);
    sk_FragColor.x = z;
}
