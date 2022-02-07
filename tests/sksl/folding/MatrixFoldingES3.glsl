
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_eq_half_b() {
    bool ok = true;
    return ok;
}
bool test_matrix_op_matrix_float_b() {
    bool ok = true;
    ok = ok && mat3x2(1.0, 4.0, 2.0, 5.0, 3.0, 6.0) * mat2x3(7.0, 9.0, 11.0, 8.0, 10.0, 12.0) == mat2(58.0, 139.0, 64.0, 154.0);
    return ok;
}
bool test_matrix_op_matrix_half_b() {
    bool ok = true;
    ok = ok && mat3x2(1.0, 4.0, 2.0, 5.0, 3.0, 6.0) * mat2x3(7.0, 9.0, 11.0, 8.0, 10.0, 12.0) == mat2(58.0, 139.0, 64.0, 154.0);
    return ok;
}
vec4 main() {
    bool _0_ok = true;
    return ((_0_ok && test_eq_half_b()) && test_matrix_op_matrix_float_b()) && test_matrix_op_matrix_half_b() ? colorGreen : colorRed;
}
