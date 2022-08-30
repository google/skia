
out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return determinant(testMatrix2x2) == -2.0 ? colorGreen : colorRed;
}
