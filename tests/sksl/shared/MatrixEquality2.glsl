
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
vec4 main() {
    bool _0_ok = true;
    mat2 _1_tempMatL;
    mat2 _2_tempMatR;
    _0_ok = _0_ok && ((_1_tempMatL = testMatrix2x2 , _2_tempMatR = mat2(1.0, 2.0, 3.0, 4.0)) , _1_tempMatL[0] == _2_tempMatR[0] && _1_tempMatL[1] == _2_tempMatR[1]);
    return _0_ok ? colorGreen : colorRed;
}
