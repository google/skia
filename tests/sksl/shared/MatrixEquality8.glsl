
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
vec4 main() {
    bool _0_ok = true;
    _0_ok = ((_0_ok && testMatrix3x3[0] == vec3(1.0, 2.0, 3.0)) && testMatrix3x3[1] == vec3(4.0, 5.0, 6.0)) && testMatrix3x3[2] == vec3(7.0, 8.0, 9.0);
    return _0_ok ? colorGreen : colorRed;
}
