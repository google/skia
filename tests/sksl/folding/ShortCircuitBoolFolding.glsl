
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
vec4 main() {
    bool _0_expr = unknownInput > 0.0;
    int _1_ok = 0;
    int _2_bad = 0;
    if (_0_expr) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    {
        ++_1_ok;
    }
    if (true ^^ _0_expr) {
        ++_2_bad;
    } else {
        ++_1_ok;
    }
    if (_0_expr) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    {
        ++_1_ok;
    }
    if (_0_expr) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    if (_0_expr) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    if (false == _0_expr) {
        ++_2_bad;
    } else {
        ++_1_ok;
    }
    if (true != _0_expr) {
        ++_2_bad;
    } else {
        ++_1_ok;
    }
    if (_0_expr) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    if (_0_expr) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    {
        ++_1_ok;
    }
    if (_0_expr ^^ true) {
        ++_2_bad;
    } else {
        ++_1_ok;
    }
    if (_0_expr) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    {
        ++_1_ok;
    }
    if (_0_expr) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    if (_0_expr) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    if (_0_expr == false) {
        ++_2_bad;
    } else {
        ++_1_ok;
    }
    if (_0_expr != true) {
        ++_2_bad;
    } else {
        ++_1_ok;
    }
    if (_0_expr) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    float _3_a = unknownInput + 2.0;
    float _4_b = unknownInput * 2.0;
    if (_3_a == _4_b) {
        ++_2_bad;
    } else {
        ++_1_ok;
    }
    bool(_3_a = _4_b) || true;
    if (_3_a == _4_b) {
        ++_1_ok;
    } else {
        ++_2_bad;
    }
    return _1_ok == 22 && _2_bad == 0 ? colorGreen : colorRed;
}
