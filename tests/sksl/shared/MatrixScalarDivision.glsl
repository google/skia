
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
vec4 main() {
    bool ok = true;
    mat3 matrixScalarDivide = testMatrix3x3 / 2.0;
    mat2 scalarMatrixDivide = 12.0 / testMatrix2x2;
    ok = ok && matrixScalarDivide == mat3(0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5);
    vec4 delta = vec4(scalarMatrixDivide[0], scalarMatrixDivide[1]) - vec4(12.0, 6.0, 4.0, 3.0);
    ok = ok && all(lessThan(abs(delta), vec4(0.0099999997764825821)));
    return ok ? colorGreen : colorRed;
}
