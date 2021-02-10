
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
vec4 main() {
    bool _1_ok = true;
    _1_ok = testMatrix2x2 == mat2(1.0, 2.0, 3.0, 4.0);
    _1_ok = _1_ok && testMatrix3x3 == mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    _1_ok = _1_ok && testMatrix2x2 != mat2(100.0);
    _1_ok = _1_ok && testMatrix3x3 != mat3(9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    return _1_ok ? colorGreen : colorRed;

}
