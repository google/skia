
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool _0_ok = true;
    mat3x4 _1_m34 = mat3x4(5.0);
    _0_ok = _0_ok && _1_m34 == mat3x4(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0);
    return _0_ok ? colorGreen : colorRed;
}
