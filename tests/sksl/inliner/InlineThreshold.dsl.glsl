
out vec4 sk_FragColor;
uniform vec4 colorGreen;
void tooBig_vi(inout int x) {
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
}
vec4 main() {
    int x = 0;
    tooBig_vi(x);
    tooBig_vi(x);
    return colorGreen;
}
