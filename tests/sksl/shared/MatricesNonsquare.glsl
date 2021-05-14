
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half_b() {
    mat2x3 m23 = mat2x3(23.0);
    mat2x4 m24 = mat2x4(24.0);
    mat3x2 m32 = mat3x2(32.0);
    mat3x4 m34 = mat3x4(34.0);
    mat4x2 m42 = mat4x2(42.0);
    mat4x3 m43 = mat4x3(43.0);
    mat2 m22 = m32 * m23;
    m22 *= m22;
    mat3 m33 = m43 * m34;
    m33 *= m33;
    mat4 m44 = m24 * m42;
    m44 *= m44;
    return true;
}
vec4 main() {
    bool _0_ok = true;
    mat2x3 _1_m23 = mat2x3(23.0);
    _0_ok = _0_ok && _1_m23 == mat2x3(23.0, 0.0, 0.0, 0.0, 23.0, 0.0);
    mat2x4 _2_m24 = mat2x4(24.0);
    _0_ok = _0_ok && _2_m24 == mat2x4(24.0, 0.0, 0.0, 0.0, 0.0, 24.0, 0.0, 0.0);
    mat3x2 _3_m32 = mat3x2(32.0);
    _0_ok = _0_ok && _3_m32 == mat3x2(32.0, 0.0, 0.0, 32.0, 0.0, 0.0);
    mat3x4 _4_m34 = mat3x4(34.0);
    _0_ok = _0_ok && _4_m34 == mat3x4(34.0, 0.0, 0.0, 0.0, 0.0, 34.0, 0.0, 0.0, 0.0, 0.0, 34.0, 0.0);
    mat4x2 _5_m42 = mat4x2(42.0);
    _0_ok = _0_ok && _5_m42 == mat4x2(42.0, 0.0, 0.0, 42.0, 0.0, 0.0, 0.0, 0.0);
    mat4x3 _6_m43 = mat4x3(43.0);
    _0_ok = _0_ok && _6_m43 == mat4x3(43.0, 0.0, 0.0, 0.0, 43.0, 0.0, 0.0, 0.0, 43.0, 0.0, 0.0, 0.0);
    mat2 _7_m22 = _3_m32 * _1_m23;
    _0_ok = _0_ok && _7_m22 == mat2(736.0);
    mat3 _8_m33 = _6_m43 * _4_m34;
    _0_ok = _0_ok && _8_m33 == mat3(1462.0);
    mat4 _9_m44 = _2_m24 * _5_m42;
    _0_ok = _0_ok && _9_m44 == mat4(1008.0, 0.0, 0.0, 0.0, 0.0, 1008.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    return _0_ok && test_half_b() ? colorGreen : colorRed;
}
