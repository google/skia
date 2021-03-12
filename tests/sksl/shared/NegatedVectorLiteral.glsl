
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_int() {
    int one = 1;
    const int two = 2;
    ivec4 result;
    result.x = 1;
    result.y = 1;
    result.z = int(-ivec4(two) == ivec4(-2, ivec3(-2)) ? 1 : 0);
    result.w = int(-ivec2(-one, one + one) == -ivec2(one - two, two) ? 1 : 0);
    return bool(((result.x * result.y) * result.z) * result.w);
}
vec4 main() {
    const float _0_one = 1.0;
    float _1_two = 2.0;
    vec4 _2_result;
    _2_result.x = 1.0;
    _2_result.y = 1.0;
    _2_result.z = float(-vec4(_1_two) == vec4(-_1_two, vec3(-_1_two)) ? 1 : 0);
    _2_result.w = float(vec2(1.0, -2.0) == -vec2(_0_one - _1_two, _1_two) ? 1 : 0);
    return bool(((_2_result.x * _2_result.y) * _2_result.z) * _2_result.w) && test_int() ? colorGreen : colorRed;
}
