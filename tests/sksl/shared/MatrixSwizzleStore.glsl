
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat3 testMatrix3x3;
uniform mat4 testMatrix4x4;
bool test4x4_b() {
    mat4 matrix;
    vec4 values = vec4(4.0, 3.0, 2.0, 1.0);
    for (int index = 0;index < 4; ++index) {
        matrix[index].wx = values.xw;
        matrix[index].zy = values.yz;
        values += 4.0;
    }
    return matrix == testMatrix4x4;
}
vec4 main() {
    mat3 _0_matrix;
    vec3 _1_values = vec3(3.0, 2.0, 1.0);
    for (int _2_index = 0;_2_index < 3; ++_2_index) {
        _0_matrix[_2_index].zx = _1_values.xz;
        _0_matrix[_2_index].y = _1_values.y;
        _1_values += 3.0;
    }
    return _0_matrix == testMatrix3x3 && test4x4_b() ? colorGreen : colorRed;
}
