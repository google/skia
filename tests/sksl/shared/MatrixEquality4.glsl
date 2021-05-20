
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
vec4 main() {
    bool _0_ok = true;
    _0_ok = (_0_ok && testMatrix2x2[0] == vec2(1.0, 2.0)) && testMatrix2x2[1] == vec2(3.0, 4.0);
    return _0_ok ? colorGreen : colorRed;
}
