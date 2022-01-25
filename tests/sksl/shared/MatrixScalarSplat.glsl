
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half_b() {
    bool ok = true;
    return ok;
}
vec4 main() {
    bool _0_ok = true;
    return _0_ok && test_half_b() ? colorGreen : colorRed;
}
