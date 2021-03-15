
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half() {
    mat2x3 m23 = mat2x3(23.0);
    mat2x4 m24 = mat2x4(24.0);
    mat3x2 m32 = mat3x2(32.0);
    mat3x4 m34 = mat3x4(34.0);
    mat4x2 m42 = mat4x2(42.0);
    mat4x3 m43 = mat4x3(44.0);
    mat2 m22 = m32 * m23;
    m22 *= m22;
    mat3 m33 = m43 * m34;
    m33 *= m33;
    mat4 m44 = m24 * m42;
    m44 *= m44;
    return true;
}
vec4 main() {
    mat2x3 _0_m23 = mat2x3(23.0);
    mat2x4 _1_m24 = mat2x4(24.0);
    mat3x2 _2_m32 = mat3x2(32.0);
    mat3x4 _3_m34 = mat3x4(34.0);
    mat4x2 _4_m42 = mat4x2(42.0);
    mat4x3 _5_m43 = mat4x3(44.0);
    mat2 _6_m22 = _2_m32 * _0_m23;
    _6_m22 *= _6_m22;
    mat3 _7_m33 = _5_m43 * _3_m34;
    _7_m33 *= _7_m33;
    mat4 _8_m44 = _1_m24 * _4_m42;
    _8_m44 *= _8_m44;
    return true && test_half() ? colorGreen : colorRed;
}
