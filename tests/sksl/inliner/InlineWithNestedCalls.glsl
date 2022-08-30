
out vec4 sk_FragColor;
float foo_hh(float x) {
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
    return x;
}
float bar_hh(float y) {
    y = foo_hh(y);
    return y;
}
void main() {
    float z = 0.0;
    bar_hh(z);
    sk_FragColor.x = z;
}
