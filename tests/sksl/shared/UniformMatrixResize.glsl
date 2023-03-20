
out vec4 sk_FragColor;
uniform mat3 testMatrix3x3;
uniform vec4 colorGreen;
uniform vec4 colorRed;
mat2 resizeMatrix_f22() {
    return mat2(testMatrix3x3);
}
vec4 main() {
    return resizeMatrix_f22() == mat2(1.0, 2.0, 4.0, 5.0) && mat3(resizeMatrix_f22()) == mat3(1.0, 2.0, 0.0, 4.0, 5.0, 0.0, 0.0, 0.0, 1.0) ? colorGreen : colorRed;
}
