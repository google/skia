
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    bool _0_ok = true;
    int _1_x = 14;
    _0_ok = _0_ok && _1_x == 14;
    _1_x = 6;
    _0_ok = _0_ok && _1_x == 6;
    _1_x = 5;
    _0_ok = _0_ok && _1_x == 5;
    _1_x = 16;
    _0_ok = _0_ok && _1_x == 16;
    _1_x = ~_1_x;
    _0_ok = _0_ok && _1_x == -17;
    _0_ok = _0_ok && _1_x == -17;
    _1_x = -8;
    _0_ok = _0_ok && _1_x == -8;
    _1_x = 32;
    _0_ok = _0_ok && _1_x == 32;
    _1_x = 33;
    _0_ok = _0_ok && _1_x == 33;
    return _0_ok ? colorGreen : colorRed;
}
