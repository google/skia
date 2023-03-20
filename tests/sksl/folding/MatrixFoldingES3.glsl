
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_eq_half_b() {
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
    const bool _0_ok = true;
    return ((((((_0_ok && test_eq_half_b()) && test_matrix_op_matrix_float_b()) && test_matrix_op_matrix_half_b()) && test_vector_op_matrix_float_b()) && test_vector_op_matrix_half_b()) && test_matrix_op_vector_float_b()) && test_matrix_op_vector_half_b() ? colorGreen : colorRed;
}
