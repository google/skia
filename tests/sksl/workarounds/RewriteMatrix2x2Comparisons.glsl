
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 colorGreen;
uniform mediump vec4 colorRed;
uniform mediump mat2 testHalf2x2;
uniform highp mat2 testFloat2x2;
mediump vec4 main() {
    mediump mat2 _tempMatrix0;
    mediump mat2 _tempMatrix1;
    highp mat2 _tempMatrix2;
    highp mat2 _tempMatrix3;
    mediump mat2 _tempMatrix4;
    mediump mat2 _tempMatrix5;
    highp mat2 _tempMatrix6;
    highp mat2 _tempMatrix7;
    bool _0_ok = true;
    _0_ok = _0_ok && ((_tempMatrix0 = testHalf2x2), (_tempMatrix1 = mat2(1.0, 2.0, 3.0, 4.0)), (_tempMatrix0[0] == _tempMatrix1[0] && _tempMatrix0[1] == _tempMatrix1[1]));
    _0_ok = _0_ok && ((_tempMatrix2 = testFloat2x2), (_tempMatrix3 = mat2(5.0, 6.0, 7.0, 8.0)), (_tempMatrix2[0] == _tempMatrix3[0] && _tempMatrix2[1] == _tempMatrix3[1]));
    _0_ok = _0_ok && ((_tempMatrix4 = testHalf2x2), (_tempMatrix5 = mat2(123.0)), !(_tempMatrix4[0] == _tempMatrix5[0] && _tempMatrix4[1] == _tempMatrix5[1]));
    _0_ok = _0_ok && ((_tempMatrix6 = testFloat2x2), (_tempMatrix7 = mat2(456.0)), !(_tempMatrix6[0] == _tempMatrix7[0] && _tempMatrix6[1] == _tempMatrix7[1]));
    return _0_ok ? colorGreen : colorRed;
}
