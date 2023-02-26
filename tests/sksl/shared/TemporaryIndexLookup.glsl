
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat3 testMatrix3x3;
mat3 GetTestMatrix_f33() {
    return testMatrix3x3;
}
vec4 main() {
    float expected = 0.0;
    for (int i = 0;i < 3; ++i) {
        for (int j = 0;j < 3; ++j) {
            expected += 1.0;
            if (GetTestMatrix_f33()[i][j] != expected) {
                return colorRed;
            }
        }
    }
    return colorGreen;
}
