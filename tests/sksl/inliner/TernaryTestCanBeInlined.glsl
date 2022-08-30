
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_bh4(vec4 v) {
    return bool(v.y);
}
vec4 main() {
    return test_bh4(colorGreen) ? colorGreen : colorRed;
}
