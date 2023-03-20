
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat3 testMatrix3x3;
uniform mat4 testMatrix4x4;
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
    mat3 _0_matrix;
    vec3 _1_values = vec3(1.0, 2.0, 3.0);
    for (int _2_index = 0;_2_index < 3; ++_2_index) {
        _0_matrix[_2_index] = _1_values;
        _1_values += 3.0;
    }
    return _0_matrix == testMatrix3x3 && test4x4_b() ? colorGreen : colorRed;
}
