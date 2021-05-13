
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half_b() {
    bool ok = true;
    mat2 m4 = mat2(6.0);
    ok = ok && m4 == mat2(6.0, 0.0, 0.0, 6.0);
    mat3 m9 = mat3(9.0);
    ok = ok && m9 == mat3(9.0, 0.0, 0.0, 0.0, 9.0, 0.0, 0.0, 0.0, 9.0);
    return ok;
}
vec4 main() {
    bool _0_ok = true;
    mat2 _1_m4 = mat2(6.0);
    _0_ok = _0_ok && _1_m4 == mat2(6.0, 0.0, 0.0, 6.0);
    mat3 _2_m9 = mat3(9.0);
    _0_ok = _0_ok && _2_m9 == mat3(9.0, 0.0, 0.0, 0.0, 9.0, 0.0, 0.0, 0.0, 9.0);
    return _0_ok && test_half_b() ? colorGreen : colorRed;
}
