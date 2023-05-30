
out vec4 sk_FragColor;
uniform vec4 colorWhite;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform mat4 testMatrix4x4;
bool test_iscalar_b() {
    int x = int(colorWhite.x);
    x = -x;
    return x == -1;
}
bool test_fvec_b() {
    vec2 x = colorWhite.xy;
    x = -x;
    return x == vec2(-1.0);
}
bool test_ivec_b() {
    ivec2 x = ivec2(int(colorWhite.x));
    x = -x;
    return x == ivec2(-1);
}
bool test_mat2_b() {
    const mat2 negated = mat2(-1.0, -2.0, -3.0, -4.0);
    mat2 x = testMatrix2x2;
    x = -x;
    return x == negated;
}
bool test_mat3_b() {
    const mat3 negated = mat3(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0);
    mat3 x = testMatrix3x3;
    x = -x;
    return x == negated;
}
bool test_mat4_b() {
    const mat4 negated = mat4(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0, -10.0, -11.0, -12.0, -13.0, -14.0, -15.0, -16.0);
    mat4 x = testMatrix4x4;
    x = -x;
    return x == negated;
}
bool test_hmat2_b() {
    const mat2 negated = mat2(-1.0, -2.0, -3.0, -4.0);
    mat2 x = testMatrix2x2;
    x = -x;
    return x == negated;
}
bool test_hmat3_b() {
    const mat3 negated = mat3(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0);
    mat3 x = testMatrix3x3;
    x = -x;
    return x == negated;
}
bool test_hmat4_b() {
    const mat4 negated = mat4(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0, -10.0, -11.0, -12.0, -13.0, -14.0, -15.0, -16.0);
    mat4 x = testMatrix4x4;
    x = -x;
    return x == negated;
}
vec4 main() {
    float _0_x = colorWhite.x;
    _0_x = -_0_x;
    return ((((((((_0_x == -1.0 && test_iscalar_b()) && test_fvec_b()) && test_ivec_b()) && test_mat2_b()) && test_mat3_b()) && test_mat4_b()) && test_hmat2_b()) && test_hmat3_b()) && test_hmat4_b() ? colorGreen : colorRed;
}
