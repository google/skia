
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
bool test_equality_b() {
    bool ok = true;
    ok = ok && testMatrix2x2 == mat2(1.0, 2.0, 3.0, 4.0);
    ok = ok && testMatrix3x3 == mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    ok = ok && testMatrix2x2 != mat2(100.0);
    ok = ok && testMatrix3x3 != mat3(9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    return ok;
}
vec4 main() {
    return test_equality_b() ? colorGreen : colorRed;
}
