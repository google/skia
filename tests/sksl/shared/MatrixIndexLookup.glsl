
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat3 testMatrix3x3;
uniform mat4 testMatrix4x4;
bool test3x3_b() {
    mat3 matrix = testMatrix3x3;
    vec3 expected = vec3(1.0, 2.0, 3.0);
    for (int index = 0;index < 3; ++index) {
        if (matrix[index] != expected) {
            return false;
        }
        expected += 3.0;
    }
    return true;
}
bool test4x4_b() {
    mat4 matrix = testMatrix4x4;
    vec4 expected = vec4(1.0, 2.0, 3.0, 4.0);
    for (int index = 0;index < 4; ++index) {
        if (matrix[index] != expected) {
            return false;
        }
        expected += 4.0;
    }
    return true;
}
vec4 main() {
    return test3x3_b() && test4x4_b() ? colorGreen : colorRed;
}
