
float _determinant2(mat2 m) {    return m[0][0] * m[1][1] - m[0][1] * m[1][0];}out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return _determinant2(testMatrix2x2) == -2.0 ? colorGreen : colorRed;
}
