
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
vec4 main() {
    bool _0_ok = true;
    _0_ok = ((((((((_0_ok && testMatrix3x3[0].x == 1.0) && testMatrix3x3[0].y == 2.0) && testMatrix3x3[0].z == 3.0) && testMatrix3x3[1].x == 4.0) && testMatrix3x3[1].y == 5.0) && testMatrix3x3[1].z == 6.0) && testMatrix3x3[2].x == 7.0) && testMatrix3x3[2].y == 8.0) && testMatrix3x3[2].z == 9.0;
    return _0_ok ? colorGreen : colorRed;
}
