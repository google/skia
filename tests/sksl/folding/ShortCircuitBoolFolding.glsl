
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
vec4 main() {
    bool _1_expr = unknownInput > 0.0;
    int _2_ok = 0;
    int _3_bad = 0;

    if (_1_expr) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    {
        ++_2_ok;
    }
    if (true ^^ _1_expr) {
        ++_3_bad;
    } else {
        ++_2_ok;
    }
    if (_1_expr) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    {
        ++_2_ok;
    }
    if (_1_expr) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    if (_1_expr) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    if (false == _1_expr) {
        ++_3_bad;
    } else {
        ++_2_ok;
    }
    if (true != _1_expr) {
        ++_3_bad;
    } else {
        ++_2_ok;
    }
    if (_1_expr) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    if (_1_expr) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    {
        ++_2_ok;
    }
    if (_1_expr ^^ true) {
        ++_3_bad;
    } else {
        ++_2_ok;
    }
    if (_1_expr) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    {
        ++_2_ok;
    }
    if (_1_expr) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    if (_1_expr) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    if (_1_expr == false) {
        ++_3_bad;
    } else {
        ++_2_ok;
    }
    if (_1_expr != true) {
        ++_3_bad;
    } else {
        ++_2_ok;
    }
    if (_1_expr) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    float _4_a = sqrt(1.0);
    float _5_b = sqrt(2.0);

    if (_4_a == _5_b) {
        ++_3_bad;
    } else {
        ++_2_ok;
    }
    bool(_4_a = _5_b) || true;
    if (_4_a == _5_b) {
        ++_2_ok;
    } else {
        ++_3_bad;
    }
    return _2_ok == 22 && _3_bad == 0 ? colorGreen : colorRed;

}
