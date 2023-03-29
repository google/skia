
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat3 testMatrix3x3;
uniform mat4 testMatrix4x4;
bool test3x3_b() {
    mat3 matrix;
    vec3 values = vec3(1.0, 2.0, 3.0);
    for (int index = 0;index < 3; ++index) {
        matrix[index] = values;
        values += 3.0;
    }
    return matrix == testMatrix3x3;
}
bool test4x4_b() {
    mat4 matrix;
    vec4 values = vec4(1.0, 2.0, 3.0, 4.0);
    for (int index = 0;index < 4; ++index) {
        matrix[index] = values;
        values += 4.0;
    }
    return matrix == testMatrix4x4;
}
vec4 main() {
    return test3x3_b() && test4x4_b() ? colorGreen : colorRed;
}
