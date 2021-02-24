
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    bool _0_test;
    bool _1_ok = true;
    int _2_x = 14;
    _1_ok = _1_ok && _2_x == 14;
    _2_x = 6;
    _1_ok = _1_ok && _2_x == 6;
    _2_x = 5;
    _1_ok = _1_ok && _2_x == 5;
    _2_x = 16;
    _1_ok = _1_ok && _2_x == 16;
    _2_x = -8;
    _1_ok = _1_ok && _2_x == -8;
    _2_x = 32;
    _1_ok = _1_ok && _2_x == 32;
    _2_x = 33;
    _1_ok = _1_ok && _2_x == 33;
    return _1_ok ? colorGreen : colorRed;

}
