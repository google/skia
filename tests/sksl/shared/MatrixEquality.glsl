
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
vec4 main() {
    bool _0_ok = true;
    _0_ok = _0_ok && testMatrix2x2 == mat2(1.0, 2.0, 3.0, 4.0);
    _0_ok = _0_ok && testMatrix3x3 == mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    _0_ok = _0_ok && testMatrix2x2 != mat2(100.0);
    _0_ok = _0_ok && testMatrix3x3 != mat3(9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    return _0_ok ? colorGreen : colorRed;
}
