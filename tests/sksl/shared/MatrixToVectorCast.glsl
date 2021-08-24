
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
vec4 main() {
    bool ok = true;
    ok = ok && vec4(testMatrix2x2) == vec4(1.0, 2.0, 3.0, 4.0);
    ok = ok && vec4(testMatrix2x2) == vec4(1.0, 2.0, 3.0, 4.0);
    ok = ok && ivec4(vec4(testMatrix2x2)) == ivec4(1, 2, 3, 4);
    ok = ok && bvec4(vec4(testMatrix2x2)) == bvec4(true, true, true, true);
    return ok ? colorGreen : colorRed;
}
