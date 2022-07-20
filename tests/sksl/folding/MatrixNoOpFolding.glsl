
out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform vec4 testInputs;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
bool test_mat3_mat3_b() {
    mat3 m;
    mat3 mm;
    const mat3 i = mat3(1.0);
    const mat3 z = mat3(0.0);
    const mat3 s = mat3(vec3(1.0), vec3(1.0), vec3(1.0));
    m = testMatrix3x3 * i;
    m = i * testMatrix3x3;
    m = m * i;
    m = i * m;
    m *= i;
    m = m / s;
    m /= s;
    m = m + z;
    m = z + m;
    m = m - z;
    m = z - m;
    mm = m * z;
    mm = z * m;
    return m == -testMatrix3x3 && mm == z;
}
bool test_mat4_mat4_b() {
    mat4 testMatrix4x4 = mat4(testInputs, testInputs, testInputs, testInputs);
    mat4 m;
    mat4 mm;
    const mat4 i = mat4(1.0);
    const mat4 z = mat4(0.0);
    const mat4 s = mat4(vec4(1.0), vec4(1.0), vec4(1.0), vec4(1.0));
    m = testMatrix4x4 * i;
    m = i * testMatrix4x4;
    m = m * i;
    m = i * m;
    m *= i;
    m = m / s;
    m /= s;
    m = m + z;
    m = z + m;
    m = m - z;
    m = z - m;
    mm = m * z;
    mm = z * m;
    return m == -testMatrix4x4 && mm == z;
}
bool test_mat2_vec2_b() {
    const mat2 i = mat2(1.0);
    const mat2 z = mat2(0.0);
    vec2 v;
    vec2 vv;
    v = testInputs.xy * i;
    v = i * testInputs.xy;
    v = v * i;
    v = i * v;
    v *= i;
    vv = v * z;
    vv = z * v;
    return v == testInputs.xy && vv == vec2(0.0, 0.0);
}
bool test_mat3_vec3_b() {
    const mat3 i = mat3(1.0);
    const mat3 z = mat3(0.0);
    vec3 v;
    vec3 vv;
    v = testInputs.xyz * i;
    v = i * testInputs.xyz;
    v = v * i;
    v = i * v;
    v *= i;
    vv = v * z;
    vv = z * v;
    return v == testInputs.xyz && vv == vec3(0.0, 0.0, 0.0);
}
vec4 main() {
    mat2 _0_m;
    mat2 _1_mm;
    const mat2 _2_i = mat2(1.0);
    const mat2 _3_z = mat2(0.0);
    const mat2 _4_s = mat2(vec4(1.0));
    _0_m = testMatrix2x2 * _2_i;
    _0_m = _2_i * testMatrix2x2;
    _0_m = _0_m * _2_i;
    _0_m = _2_i * _0_m;
    _0_m *= _2_i;
    _0_m = _0_m / _4_s;
    _0_m /= _4_s;
    _0_m = _0_m + _3_z;
    _0_m = _3_z + _0_m;
    _0_m = _0_m - _3_z;
    _0_m = _3_z - _0_m;
    _1_mm = _0_m * _3_z;
    _1_mm = _3_z * _0_m;
    return ((((_0_m == -testMatrix2x2 && _1_mm == _3_z) && test_mat3_mat3_b()) && test_mat4_mat4_b()) && test_mat2_vec2_b()) && test_mat3_vec3_b() ? colorGreen : colorRed;
}
