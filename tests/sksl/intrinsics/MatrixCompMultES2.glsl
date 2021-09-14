
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
vec4 main() {
    mat2 h22 = mat2(0.0, 5.0, 10.0, 15.0);
    mat2 f22 = matrixCompMult(testMatrix2x2, mat2(1.0));
    mat3 h33 = matrixCompMult(testMatrix3x3, mat3(2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0));
    return (h22 == mat2(0.0, 5.0, 10.0, 15.0) && f22 == mat2(1.0, 0.0, 0.0, 4.0)) && h33 == mat3(2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0) ? colorGreen : colorRed;
}
