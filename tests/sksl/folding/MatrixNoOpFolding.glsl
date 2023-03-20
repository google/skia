
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform vec4 testInputs;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
bool test_mat3_mat3_b() {
    mat3 m;
    mat3 mm;
    const mat3 z = mat3(0.0);
    m = testMatrix3x3;
    m = testMatrix3x3;
    m = -m;
    mm = mat3(0.0);
    mm = mat3(0.0);
    return m == -testMatrix3x3 && mm == z;
}
bool test_mat4_mat4_b() {
    mat4 testMatrix4x4 = mat4(testInputs, testInputs, testInputs, testInputs);
    mat4 m;
    mat4 mm;
    const mat4 z = mat4(0.0);
    m = testMatrix4x4;
    m = testMatrix4x4;
    m = -m;
    mm = mat4(0.0);
    mm = mat4(0.0);
    return m == -testMatrix4x4 && mm == z;
}
vec4 main() {
    mat2 _0_m;
    mat2 _1_mm;
    const mat2 _3_z = mat2(0.0);
    _0_m = testMatrix2x2;
    _0_m = testMatrix2x2;
    _0_m = -_0_m;
    _1_mm = mat2(0.0);
    _1_mm = mat2(0.0);
    return ((_0_m == -testMatrix2x2 && _1_mm == _3_z) && test_mat3_mat3_b()) && test_mat4_mat4_b() ? colorGreen : colorRed;
}
