
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half_b() {
    bool ok = true;
    mat2 m7 = mat2(5.0, 6.0, 7.0, 8.0);
    ok = ok && m7 == mat2(5.0, 6.0, 7.0, 8.0);
    return ok;
}
vec4 main() {
    bool _0_ok = true;
    mat2 _1_m7 = mat2(5.0, 6.0, 7.0, 8.0);
    _0_ok = _0_ok && _1_m7 == mat2(5.0, 6.0, 7.0, 8.0);
    return _0_ok && test_half_b() ? colorGreen : colorRed;
}
