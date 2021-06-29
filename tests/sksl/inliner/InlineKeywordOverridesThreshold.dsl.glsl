
out vec4 sk_FragColor;
uniform vec4 colorGreen;
int tooBig_ii(int x) {
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
    return x;
}
vec4 main() {
    int y = 0;
    y = tooBig_ii(y);
    y = tooBig_ii(y);
    return colorGreen;
}
