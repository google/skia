
uniform mat2 testMatrix2x2;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
bool test_matrix_op_scalar_float_b() {
    bool ok = true;
    return ok;
}
bool test_matrix_op_scalar_half_b() {
    bool ok = true;
    return ok;
}
bool test_matrix_op_matrix_float_b() {
    bool ok = true;
    return ok;
}
bool test_matrix_op_matrix_half_b() {
    bool ok = true;
    return ok;
}
bool test_vector_op_matrix_float_b() {
    bool ok = true;
    return ok;
}
bool test_vector_op_matrix_half_b() {
    bool ok = true;
    return ok;
}
bool test_matrix_op_vector_float_b() {
    bool ok = true;
    return ok;
}
bool test_matrix_op_vector_half_b() {
    bool ok = true;
    return ok;
}
vec4 main() {
    bool _0_ok = true;
    _0_ok = _0_ok && mat3(unknownInput) == mat3(mat2(1.0));
    _0_ok = _0_ok && mat3(9.0, 0.0, 0.0, 0.0, 9.0, 0.0, 0.0, 0.0, unknownInput) == mat3(mat2(9.0));
    _0_ok = _0_ok && vec4(testMatrix2x2) == vec4(1.0, 2.0, 3.0, 4.0);
    {
        _0_ok = _0_ok && mat4(mat3(testMatrix2x2))[0] == vec4(1.0, 2.0, 0.0, 0.0);
        _0_ok = _0_ok && mat4(mat3(testMatrix2x2))[1] == vec4(3.0, 4.0, 0.0, 0.0);
    }
    return (((((((_0_ok && test_matrix_op_scalar_float_b()) && test_matrix_op_scalar_half_b()) && test_matrix_op_matrix_float_b()) && test_matrix_op_matrix_half_b()) && test_vector_op_matrix_float_b()) && test_vector_op_matrix_half_b()) && test_matrix_op_vector_float_b()) && test_matrix_op_vector_half_b() ? colorGreen : colorRed;
}
