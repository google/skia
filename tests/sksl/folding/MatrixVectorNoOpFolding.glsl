
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform vec4 testInputs;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
bool test_Xno_Xop_Xmat2_XX_Xvec2_Xb() {
    vec2 v;
    vec2 vv;
    v = testInputs.xy;
    v = testInputs.xy;
    if (v != testInputs.xy) return false;
    if (v != testInputs.xy) return false;
    v = -testInputs.xy;
    v = -testInputs.xy;
    if (v != -testInputs.xy) return false;
    vv = vec2(0.0);
    vv = vec2(0.0);
    return vv == vec2(0.0);
}
bool test_Xno_Xop_Xmat3_XX_Xvec3_Xb() {
    vec3 v;
    vec3 vv;
    v = testInputs.xyz;
    v = testInputs.xyz;
    if (v != testInputs.xyz) return false;
    if (v != testInputs.xyz) return false;
    v = -testInputs.xyz;
    v = -testInputs.xyz;
    if (v != -testInputs.xyz) return false;
    vv = vec3(0.0);
    vv = vec3(0.0);
    return vv == vec3(0.0);
}
bool test_Xno_Xop_Xmat4_XX_Xvec4_Xb() {
    vec4 v;
    vec4 vv;
    v = testInputs;
    v = testInputs;
    if (v != testInputs) return false;
    if (v != testInputs) return false;
    v = -testInputs;
    v = -testInputs;
    if (v != -testInputs) return false;
    vv = vec4(0.0);
    vv = vec4(0.0);
    return vv == vec4(0.0);
}
bool test_Xno_Xop_Xvec2_XX_Xmat2_Xb() {
    const vec2 n = vec2(-1.0);
    const vec2 i = vec2(1.0);
    const vec2 z = vec2(0.0);
    vec2 v;
    vec2 vv = vec2(0.0);
    vv = vec2(0.0);
    if (vv != z) return false;
    v = i * testMatrix2x2;
    if (v != vec2(3.0, 7.0)) return false;
    v = testMatrix2x2 * i;
    if (v != vec2(4.0, 6.0)) return false;
    v = n * testMatrix2x2;
    if (v != vec2(-3.0, -7.0)) return false;
    v = testMatrix2x2 * n;
    return v == vec2(-4.0, -6.0);
}
bool test_Xno_Xop_Xvec3_XX_Xmat3_Xb() {
    const vec3 n = vec3(-1.0);
    const vec3 i = vec3(1.0);
    const vec3 z = vec3(0.0);
    vec3 v;
    vec3 vv = vec3(0.0);
    vv = vec3(0.0);
    if (vv != z) return false;
    v = i * testMatrix3x3;
    if (v != vec3(6.0, 15.0, 24.0)) return false;
    v = testMatrix3x3 * i;
    if (v != vec3(12.0, 15.0, 18.0)) return false;
    v = n * testMatrix3x3;
    if (v != vec3(-6.0, -15.0, -24.0)) return false;
    v = testMatrix3x3 * n;
    return v == vec3(-12.0, -15.0, -18.0);
}
bool test_Xno_Xop_Xvec4_XX_Xmat4_Xb() {
    const vec4 n = vec4(-1.0);
    const vec4 i = vec4(1.0);
    const vec4 z = vec4(0.0);
    mat4 testMatrix4x4 = mat4(testMatrix2x2[0], testMatrix2x2[1], testMatrix2x2[0], testMatrix2x2[1], testMatrix2x2[0], testMatrix2x2[1], testMatrix2x2[0], testMatrix2x2[1]);
    vec4 v;
    vec4 vv = vec4(0.0);
    vv = vec4(0.0);
    if (vv != z) return false;
    v = i * testMatrix4x4;
    if (v != vec4(10.0)) return false;
    v = testMatrix4x4 * i;
    if (v != vec4(4.0, 8.0, 12.0, 16.0)) return false;
    v = n * testMatrix4x4;
    if (v != vec4(-10.0)) return false;
    v = testMatrix4x4 * n;
    return v == vec4(-4.0, -8.0, -12.0, -16.0);
}
vec4 main() {
    return ((((test_Xno_Xop_Xmat2_XX_Xvec2_Xb() && test_Xno_Xop_Xmat3_XX_Xvec3_Xb()) && test_Xno_Xop_Xmat4_XX_Xvec4_Xb()) && test_Xno_Xop_Xvec2_XX_Xmat2_Xb()) && test_Xno_Xop_Xvec3_XX_Xmat3_Xb()) && test_Xno_Xop_Xvec4_XX_Xmat4_Xb() ? colorGreen : colorRed;
}
