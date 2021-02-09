
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_float() {
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
    return test_float() && test_half() ? colorGreen : colorRed;
}
