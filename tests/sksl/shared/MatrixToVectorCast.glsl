
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
vec4 main() {
    bool ok = true;
    ok = ok && vec4(testMatrix2x2) == vec4(1.0, 2.0, 3.0, 4.0);
    ok = ok && vec4(testMatrix2x2) == vec4(1.0, 2.0, 3.0, 4.0);
    return ok ? colorGreen : colorRed;
}
