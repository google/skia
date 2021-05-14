
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool _0_ok = true;
    mat2x3 _1_m23 = mat2x3(2.0);
    _0_ok = _0_ok && _1_m23 == mat2x3(2.0, 0.0, 0.0, 0.0, 2.0, 0.0);
    return _0_ok ? colorGreen : colorRed;
}
