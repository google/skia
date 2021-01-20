
vec4 main() {
    bool _1_expr1 = sqrt(1.0) > 0.0;
    bool _2_expr2 = sqrt(2.0) > 0.0;
    int _3_ok = 0;
    int _4_bad = 0;

    if (_1_expr1) {
        ++_3_ok;
    } else {
        ++_4_bad;
    }
    {
        ++_3_ok;
    }
    if (true ^^ _1_expr1) {
        ++_3_ok;
    } else {
        ++_4_bad;
    }
    if (_2_expr2) {
        ++_4_bad;
    } else {
        ++_3_ok;
    }
    {
        ++_3_ok;
    }
    if (_2_expr2) {
        ++_3_ok;
    } else {
        ++_4_bad;
    }
    if (_1_expr1) {
        ++_3_ok;
    } else {
        ++_4_bad;
    }
    {
        ++_3_ok;
    }
    if (_1_expr1 ^^ true) {
        ++_3_ok;
    } else {
        ++_4_bad;
    }
    if (_2_expr2) {
        ++_4_bad;
    } else {
        ++_3_ok;
    }
    {
        ++_3_ok;
    }
    if (_2_expr2) {
        ++_3_ok;
    } else {
        ++_4_bad;
    }
    return _3_ok == 12 && _4_bad == 0 ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);

}
