
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform vec4 testInputs;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
bool test_Xno_Xop_Xscalar_XX_Xmat2_Xb() {
    mat2 m;
    mat2 mm;
    const mat2 z = mat2(0.0);
    m = testMatrix2x2;
    m = testMatrix2x2;
    if (m != testMatrix2x2) return false;
    if (m != testMatrix2x2) return false;
    if (m != testMatrix2x2) return false;
    m = -m;
    if (m != -testMatrix2x2) return false;
    mm = mat2(0.0);
    mm = mat2(0.0);
    return mm == z;
}
bool test_Xno_Xop_Xscalar_XX_Xmat3_Xb() {
    mat3 m;
    mat3 mm;
    const mat3 z = mat3(0.0);
    m = testMatrix3x3;
    m = testMatrix3x3;
    if (m != testMatrix3x3) return false;
    if (m != testMatrix3x3) return false;
    if (m != testMatrix3x3) return false;
    m = -m;
    if (m != -testMatrix3x3) return false;
    mm = mat3(0.0);
    mm = mat3(0.0);
    return mm == z;
}
bool test_Xno_Xop_Xscalar_XX_Xmat4_Xb() {
    mat4 testMatrix4x4 = mat4(testInputs, testInputs, testInputs, testInputs);
    mat4 m;
    mat4 mm;
    const mat4 z = mat4(0.0);
    m = testMatrix4x4;
    m = testMatrix4x4;
    if (m != testMatrix4x4) return false;
    if (m != testMatrix4x4) return false;
    if (m != testMatrix4x4) return false;
    m = -m;
    if (m != -testMatrix4x4) return false;
    mm = mat4(0.0);
    mm = mat4(0.0);
    return mm == z;
}
bool test_Xno_Xop_Xmat2_XX_Xscalar_Xb() {
    mat2 m;
    mat2 mm;
    const mat2 z = mat2(0.0);
    const mat2 s = mat2(vec4(1.0).xy, vec4(1.0).zw);
    float scalar = testInputs.x;
    m = mat2(scalar);
    m = mat2(scalar);
    if (m != mat2(scalar)) return false;
    m = scalar / s;
    if (m != mat2(scalar, scalar, scalar, scalar)) return false;
    m = scalar + z;
    m = z + scalar;
    if (m != mat2(scalar, scalar, scalar, scalar)) return false;
    m = scalar - z;
    m = z - scalar;
    if (m != -mat2(scalar, scalar, scalar, scalar)) return false;
    mm = mat2(0.0);
    mm = mat2(0.0);
    return mm == z;
}
bool test_Xno_Xop_Xmat3_XX_Xscalar_Xb() {
    mat3 m;
    mat3 mm;
    const mat3 z = mat3(0.0);
    const mat3 s = mat3(vec3(1.0), vec3(1.0), vec3(1.0));
    float scalar = testInputs.x;
    vec3 scalar3 = vec3(scalar);
    m = mat3(scalar);
    m = mat3(scalar);
    if (m != mat3(scalar)) return false;
    m = scalar / s;
    if (m != mat3(scalar3, scalar3, scalar3)) return false;
    m = scalar + z;
    m = z + scalar;
    if (m != mat3(scalar3, scalar3, scalar3)) return false;
    m = scalar - z;
    m = z - scalar;
    if (m != -mat3(scalar3, scalar3, scalar3)) return false;
    mm = mat3(0.0);
    mm = mat3(0.0);
    return mm == z;
}
bool test_Xno_Xop_Xmat4_XX_Xscalar_Xb() {
    mat4 m;
    mat4 mm;
    const mat4 z = mat4(0.0);
    const mat4 s = mat4(vec4(1.0), vec4(1.0), vec4(1.0), vec4(1.0));
    float scalar = testInputs.x;
    vec4 scalar4 = vec4(scalar);
    m = mat4(scalar);
    m = mat4(scalar);
    if (m != mat4(scalar)) return false;
    m = scalar / s;
    if (m != mat4(scalar4, scalar4, scalar4, scalar4)) return false;
    m = scalar + z;
    m = z + scalar;
    if (m != mat4(scalar4, scalar4, scalar4, scalar4)) return false;
    m = scalar - z;
    m = z - scalar;
    if (m != -mat4(scalar4, scalar4, scalar4, scalar4)) return false;
    mm = mat4(0.0);
    mm = mat4(0.0);
    return mm == z;
}
vec4 main() {
    return ((((test_Xno_Xop_Xscalar_XX_Xmat2_Xb() && test_Xno_Xop_Xscalar_XX_Xmat3_Xb()) && test_Xno_Xop_Xscalar_XX_Xmat4_Xb()) && test_Xno_Xop_Xmat2_XX_Xscalar_Xb()) && test_Xno_Xop_Xmat3_XX_Xscalar_Xb()) && test_Xno_Xop_Xmat4_XX_Xscalar_Xb() ? colorGreen : colorRed;
}
