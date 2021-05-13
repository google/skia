
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half_b() {
    bool ok = true;
    mat2 m1 = mat2(1.0, 2.0, 3.0, 4.0);
    ok = ok && m1 == mat2(1.0, 2.0, 3.0, 4.0);
    mat2 m5 = mat2(m1[1].y);
    ok = ok && m5 == mat2(4.0, 0.0, 0.0, 4.0);
    return ok;
}
vec4 main() {
    bool _0_ok = true;
    mat2 _1_m1 = mat2(1.0, 2.0, 3.0, 4.0);
    _0_ok = _0_ok && _1_m1 == mat2(1.0, 2.0, 3.0, 4.0);
    mat2 _2_m5 = mat2(_1_m1[1].y);
    _0_ok = _0_ok && _2_m5 == mat2(4.0, 0.0, 0.0, 4.0);
    return _0_ok && test_half_b() ? colorGreen : colorRed;
}
