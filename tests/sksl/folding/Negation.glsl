
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_ivec_b() {
    int one = 1;
    const int two = 2;
    bool ok = true;
    ok = ok && -ivec2(-one, one + one) == -ivec2(one - two, 2);
    ok = ok && -1 * one == -one;
    ok = ok && -int(colorGreen.y) == -1 * int(colorGreen.y);
    ok = ok && ivec4(colorGreen) * -1 == -ivec4(colorGreen);
    return ok;
}
bool test_mat_b() {
    bool ok = true;
    return ok;
}
vec4 main() {
    const float _0_one = 1.0;
    float _1_two = 2.0;
    bool _4_ok = true;
    _4_ok = _4_ok && -vec4(_1_two) == vec4(-_1_two, vec3(-_1_two));
    _4_ok = _4_ok && vec2(1.0, -2.0) == -vec2(_0_one - _1_two, _1_two);
    _4_ok = _4_ok && -_1_two == -1.0 * _1_two;
    _4_ok = _4_ok && colorGreen.y * -1.0 == -colorGreen.y;
    _4_ok = _4_ok && -colorGreen == -1.0 * colorGreen;
    return (_4_ok && test_ivec_b()) && test_mat_b() ? colorGreen : colorRed;
}
