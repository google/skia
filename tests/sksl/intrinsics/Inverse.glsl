
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const mat2 matrix2x2 = mat2(1.0, 2.0, 3.0, 4.0);
    mat2 inv2x2 = mat2(-2.0, 1.0, 1.5, -0.5);
    mat3 inv3x3 = mat3(-24.0, 18.0, 5.0, 20.0, -15.0, -4.0, -5.0, 4.0, 1.0);
    mat4 inv4x4 = mat4(-2.0, -0.5, 1.0, 0.5, 1.0, 0.5, 0.0, -0.5, -8.0, -1.0, 2.0, 2.0, 3.0, 0.5, -1.0, -0.5);
    float Zero = colorGreen.z;
    return (((((mat2(-2.0, 1.0, 1.5, -0.5) == inv2x2 && mat3(-24.0, 18.0, 5.0, 20.0, -15.0, -4.0, -5.0, 4.0, 1.0) == inv3x3) && mat4(-2.0, -0.5, 1.0, 0.5, 1.0, 0.5, 0.0, -0.5, -8.0, -1.0, 2.0, 2.0, 3.0, 0.5, -1.0, -0.5) == inv4x4) && inverse(mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0)) != inv3x3) && inverse(matrix2x2 + Zero) == inv2x2) && inverse(mat3(1.0, 2.0, 3.0, 0.0, 1.0, 4.0, 5.0, 6.0, 0.0) + Zero) == inv3x3) && inverse(mat4(1.0, 0.0, 0.0, 1.0, 0.0, 2.0, 1.0, 2.0, 2.0, 1.0, 0.0, 1.0, 2.0, 0.0, 1.0, 4.0) + Zero) == inv4x4 ? colorGreen : colorRed;
}
