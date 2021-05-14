
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool _0_ok = true;
    mat2x3 _1_m23 = mat2x3(2.0);
    mat3x2 _3_m32 = mat3x2(4.0);
    mat3x4 _4_m34 = mat3x4(5.0);
    mat4x3 _6_m43 = mat4x3(7.0);
    mat2 _7_m22 = _3_m32 * _1_m23;
    _0_ok = _0_ok && _7_m22 == mat2(8.0);
    mat3 _8_m33 = _6_m43 * _4_m34;
    _0_ok = _0_ok && _8_m33 == mat3(35.0);
    return _0_ok ? colorGreen : colorRed;
}
