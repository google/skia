
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_int() {
    int one = 1;
    int two = 2;
    ivec4 result;
    result.x = int(ivec4(-1) == -ivec4(-ivec2(-1), ivec2(1)) ? 1 : 0);
    result.y = int(ivec4(1) != -ivec4(1) ? 1 : 0);
    result.z = int(-ivec4(two) == ivec4(-two, ivec3(-two)) ? 1 : 0);
    result.w = int(-ivec2(-one, one + one) == -ivec2(one - two, two) ? 1 : 0);
    return bool(((result.x * result.y) * result.z) * result.w);
}
vec4 main() {
    bool _0_test_float;
    float _1_one = 1.0;
    float _2_two = 2.0;
    vec4 _3_result;
    _3_result.x = float(vec4(-1.0) == -vec4(-vec2(-1.0), vec2(1.0)) ? 1 : 0);
    _3_result.y = float(vec4(1.0) != -vec4(1.0) ? 1 : 0);
    _3_result.z = float(-vec4(_2_two) == vec4(-_2_two, vec3(-_2_two)) ? 1 : 0);
    _3_result.w = float(-vec2(-_1_one, _1_one + _1_one) == -vec2(_1_one - _2_two, _2_two) ? 1 : 0);
    return bool(((_3_result.x * _3_result.y) * _3_result.z) * _3_result.w) && test_int() ? colorGreen : colorRed;

}
