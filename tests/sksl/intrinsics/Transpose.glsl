
out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    mat2x3 testMatrix2x3 = mat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    return (transpose(testMatrix2x2) == mat2(1.0, 3.0, 2.0, 4.0) && transpose(testMatrix2x3) == mat3x2(1.0, 4.0, 2.0, 5.0, 3.0, 6.0)) && transpose(testMatrix3x3) == mat3(1.0, 4.0, 7.0, 2.0, 5.0, 8.0, 3.0, 6.0, 9.0) ? colorGreen : colorRed;
}
