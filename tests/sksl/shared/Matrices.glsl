
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half_b() {
    bool ok = true;
    return ok;
}
vec4 main() {
    bool _0_ok = true;
    mat2 _1_m1 = mat2(1.0, 2.0, 3.0, 4.0);
    _0_ok = _0_ok && _1_m1 == mat2(1.0, 2.0, 3.0, 4.0);
    mat2 _2_m2 = mat2(vec4(5.0));
    _0_ok = _0_ok && _2_m2 == mat2(5.0, 5.0, 5.0, 5.0);
    mat2 _3_m3 = _1_m1;
    _0_ok = _0_ok && _3_m3 == mat2(1.0, 2.0, 3.0, 4.0);
    mat2 _4_m4 = mat2(6.0);
    _0_ok = _0_ok && _4_m4 == mat2(6.0, 0.0, 0.0, 6.0);
    return _0_ok && test_half_b() ? colorGreen : colorRed;
}
