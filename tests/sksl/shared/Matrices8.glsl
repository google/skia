
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half_b() {
    bool ok = true;
    mat4 m10 = mat4(11.0);
    ok = ok && m10 == mat4(11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0);
    return ok;
}
vec4 main() {
    bool _0_ok = true;
    mat4 _1_m10 = mat4(11.0);
    _0_ok = _0_ok && _1_m10 == mat4(11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0);
    return _0_ok && test_half_b() ? colorGreen : colorRed;
}
