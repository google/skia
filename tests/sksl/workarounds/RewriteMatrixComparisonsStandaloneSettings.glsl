
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testHalf2x2;
uniform mat2 testFloat2x2;
uniform mat3 testHalf3x3;
uniform mat4x2 testFloat4x2;
bool test_equality_b() {
    bool ok = true;
    ok = ok && testHalf2x2 == mat2(1.0, 2.0, 3.0, 4.0);
    ok = ok && testFloat2x2 == mat2(5.0, 6.0, 7.0, 8.0);
    ok = ok && testHalf2x2 != mat2(123.0);
    ok = ok && testFloat2x2 != mat2(456.0);
    ok = ok && testHalf3x3 == mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    ok = ok && testFloat4x2 != mat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    return ok;
}
vec4 main() {
    return test_equality_b() ? colorGreen : colorRed;
}
