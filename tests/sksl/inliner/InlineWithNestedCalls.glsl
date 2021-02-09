
out vec4 sk_FragColor;
void foo(out float x) {
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    ++x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    --x;
    x = 42.0;
}
float bar(float y) {
    foo(y);
    return y;
}
void main() {
    float _1_y = 123.0;
    float z = 0.0;
    bar(z);
    sk_FragColor.x = z;
}
