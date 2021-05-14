
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool _0_ok = true;
    mat3x2 _1_m32 = mat3x2(4.0);
    _0_ok = _0_ok && _1_m32 == mat3x2(4.0, 0.0, 0.0, 4.0, 0.0, 0.0);
    return _0_ok ? colorGreen : colorRed;
}
