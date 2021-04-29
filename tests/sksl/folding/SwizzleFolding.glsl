
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    const vec4 _0_colorWhite = vec4(1.0);
    const vec2 _1_point = vec2(40.0, 60.0);
    bool _2_ok = true;
    _2_ok = _2_ok && (((_1_point.x >= 0.0 && _1_point.x <= 100.0) && _1_point.y >= 0.0) && _1_point.y <= 100.0);
    _2_ok = _2_ok && _0_colorWhite.x == 1.0;
    _2_ok = _2_ok && _0_colorWhite.x + _0_colorWhite.y == 2.0;
    _2_ok = _2_ok && (_0_colorWhite.x + _0_colorWhite.y) + _0_colorWhite.z == 3.0;
    _2_ok = _2_ok && ((_0_colorWhite.x + _0_colorWhite.y) + _0_colorWhite.z) + _0_colorWhite.w == 4.0;
    _2_ok = _2_ok && colorGreen * _0_colorWhite.x != colorRed * _0_colorWhite.y;
    const vec2 _3_pointOffset = _1_point.yx + _0_colorWhite.xz;
    _2_ok = _2_ok && _3_pointOffset == vec2(61.0, 41.0);
    return _2_ok ? colorGreen : colorRed;
}
