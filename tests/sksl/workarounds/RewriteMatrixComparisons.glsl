
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 colorGreen;
uniform mediump vec4 colorRed;
uniform mediump mat2 testHalf2x2;
uniform highp mat2 testFloat2x2;
uniform mediump mat3 testHalf3x3;
uniform highp mat4x2 testFloat4x2;
bool test_equality_b() {
    mediump mat2 _tempMatrix0;
    mediump mat2 _tempMatrix1;
    highp mat2 _tempMatrix2;
    highp mat2 _tempMatrix3;
    mediump mat2 _tempMatrix4;
    mediump mat2 _tempMatrix5;
    highp mat2 _tempMatrix6;
    highp mat2 _tempMatrix7;
    mediump mat3 _tempMatrix8;
    mediump mat3 _tempMatrix9;
    highp mat4x2 _tempMatrix10;
    highp mat4x2 _tempMatrix11;
    bool ok = true;
    ok = ok && ((_tempMatrix0 = testHalf2x2), (_tempMatrix1 = mat2(1.0, 2.0, 3.0, 4.0)), (_tempMatrix0 == _tempMatrix1));
    ok = ok && ((_tempMatrix2 = testFloat2x2), (_tempMatrix3 = mat2(5.0, 6.0, 7.0, 8.0)), (_tempMatrix2 == _tempMatrix3));
    ok = ok && ((_tempMatrix4 = testHalf2x2), (_tempMatrix5 = mat2(123.0)), (_tempMatrix4 != _tempMatrix5));
    ok = ok && ((_tempMatrix6 = testFloat2x2), (_tempMatrix7 = mat2(456.0)), (_tempMatrix6 != _tempMatrix7));
    ok = ok && ((_tempMatrix8 = testHalf3x3), (_tempMatrix9 = mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0)), (_tempMatrix8 == _tempMatrix9));
    ok = ok && ((_tempMatrix10 = testFloat4x2), (_tempMatrix11 = mat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0)), (_tempMatrix10 != _tempMatrix11));
    return ok;
}
mediump vec4 main() {
    return test_equality_b() ? colorGreen : colorRed;
}
