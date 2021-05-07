
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    const float _0_floatOne = 1.0;
    const int _1_intOne = 1;
    const vec4 _2_half4One = vec4(1.0);
    const ivec4 _3_int4One = ivec4(1);
    bool _4_ok = true;
    _4_ok = _4_ok && int(_0_floatOne) == _1_intOne;
    _4_ok = _4_ok && float(_1_intOne) == _0_floatOne;
    _4_ok = _4_ok && ivec4(_2_half4One) == _3_int4One;
    _4_ok = _4_ok && vec4(_3_int4One) == _2_half4One;
    _4_ok = _4_ok && ivec4(_2_half4One) == ivec4(1);
    _4_ok = _4_ok && vec4(_3_int4One) == vec4(_0_floatOne);
    _4_ok = _4_ok && vec4(float(_1_intOne)) == vec4(1.0);
    return _4_ok ? colorGreen : colorRed;
}
