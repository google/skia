
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool _0_ok = true;
    mat2x3 _1_m23 = mat2x3(2.0);
    _0_ok = _0_ok && _1_m23[0].x == 2.0;
    mat2x4 _2_m24 = mat2x4(3.0);
    _0_ok = _0_ok && _2_m24[0].x == 3.0;
    mat3x2 _3_m32 = mat3x2(4.0);
    _0_ok = _0_ok && _3_m32[0].x == 4.0;
    mat3x4 _4_m34 = mat3x4(5.0);
    _0_ok = _0_ok && _4_m34[0].x == 5.0;
    mat4x2 _5_m42 = mat4x2(6.0);
    _0_ok = _0_ok && _5_m42[0].x == 6.0;
    mat4x3 _6_m43 = mat4x3(7.0);
    _0_ok = _0_ok && _6_m43[0].x == 7.0;
    return _0_ok ? colorGreen : colorRed;
}
