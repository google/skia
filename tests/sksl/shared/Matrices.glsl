
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
vec4 main() {
    mat2 _1_m3 = mat2(1.0, 2.0, 3.0, 4.0);
    _1_m3 *= mat2(1.0);
    mat2 _2_m5 = mat2(mat2(1.0, 2.0, 3.0, 4.0)[0].x);
    mat2 _3_m6 = mat2(1.0, 2.0, 3.0, 4.0);
    _3_m6 += _2_m5;
    mat4 _4_m11 = mat4(2.0);
    _4_m11 -= mat4(1.0);
    mat2 _6_m3 = mat2(1.0, 2.0, 3.0, 4.0);
    _6_m3 *= mat2(1.0);
    mat2 _7_m5 = mat2(mat2(1.0, 2.0, 3.0, 4.0)[0].x);
    mat2 _8_m6 = mat2(1.0, 2.0, 3.0, 4.0);
    _8_m6 += _7_m5;
    mat4 _9_m11 = mat4(2.0);
    _9_m11 -= mat4(1.0);
    bool _11_ok = true;
    _11_ok = testMatrix2x2 == mat2(1.0, 2.0, 3.0, 4.0);
    _11_ok = _11_ok && testMatrix3x3 == mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    _11_ok = _11_ok && testMatrix2x2 != mat2(100.0);
    _11_ok = _11_ok && testMatrix3x3 != mat3(9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    return _11_ok ? colorGreen : colorRed;



}
