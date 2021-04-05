
out vec4 sk_FragColor;
uniform float unknownInput;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_int_b() {
    int unknown = int(unknownInput);
    bool ok = true;
    ok = ok && ivec4(ivec2(1), 2, 3) + ivec4(5, 6, 7, 8) == ivec4(6, 7, 9, 11);
    ok = ok && ivec4(8, ivec3(10)) - ivec4(1) == ivec4(7, 9, 9, 9);
    ok = ok && ivec4(2) * ivec4(1, 2, 3, 4) == ivec4(2, 4, 6, 8);
    ok = ok && ivec4(12) / ivec4(1, 2, 3, 4) == ivec4(12, 6, 4, 3);
    ok = ok && ivec2(1) == ivec2(1, 1);
    ok = ok && !(ivec2(1) == ivec2(1, 0));
    ok = ok && ivec4(1) == ivec4(ivec2(1), ivec2(1));
    ok = ok && ivec4(ivec3(1), 1) == ivec4(ivec2(1), ivec2(1));
    ok = ok && !(ivec4(ivec3(1), 1) == ivec4(ivec2(1), 1, 0));
    ok = ok && ivec2(1) != ivec2(1, 0);
    ok = ok && !(ivec4(1) != ivec4(ivec2(1), ivec2(1)));
    ivec4 val = ivec4(unknown);
    val += ivec4(1);
    val -= ivec4(1);
    val = val + ivec4(1);
    val = val - ivec4(1);
    ok = ok && val == ivec4(unknown);
    val *= ivec4(2);
    val /= ivec4(2);
    val = val * ivec4(2);
    val = val / ivec4(2);
    ok = ok && val == ivec4(unknown);
    return ok;
}
vec4 main() {
    float _0_unknown = unknownInput;
    bool _1_ok = true;
    _1_ok = _1_ok && vec4(vec2(1.0), 2.0, 3.0) + vec4(5.0, 6.0, 7.0, 8.0) == vec4(6.0, 7.0, 9.0, 11.0);
    _1_ok = _1_ok && vec4(8.0, vec3(10.0)) - vec4(1.0) == vec4(7.0, 9.0, 9.0, 9.0);
    _1_ok = _1_ok && vec4(2.0) * vec4(1.0, 2.0, 3.0, 4.0) == vec4(2.0, 4.0, 6.0, 8.0);
    _1_ok = _1_ok && vec4(12.0) / vec4(1.0, 2.0, 3.0, 4.0) == vec4(12.0, 6.0, 4.0, 3.0);
    _1_ok = _1_ok && vec2(1.0) == vec2(1.0, 1.0);
    _1_ok = _1_ok && !(vec2(1.0) == vec2(1.0, 0.0));
    _1_ok = _1_ok && vec4(1.0) == vec4(vec2(1.0), vec2(1.0));
    _1_ok = _1_ok && vec4(vec3(1.0), 1.0) == vec4(vec2(1.0), vec2(1.0));
    _1_ok = _1_ok && !(vec4(vec3(1.0), 1.0) == vec4(vec2(1.0), 1.0, 0.0));
    _1_ok = _1_ok && vec2(1.0) != vec2(1.0, 0.0);
    _1_ok = _1_ok && !(vec4(1.0) != vec4(vec2(1.0), vec2(1.0)));
    vec4 _2_val = vec4(_0_unknown);
    _2_val += vec4(1.0);
    _2_val -= vec4(1.0);
    _2_val = _2_val + vec4(1.0);
    _2_val = _2_val - vec4(1.0);
    _1_ok = _1_ok && _2_val == vec4(_0_unknown);
    _2_val *= vec4(2.0);
    _2_val /= vec4(2.0);
    _2_val = _2_val * vec4(2.0);
    _2_val = _2_val / vec4(2.0);
    _1_ok = _1_ok && _2_val == vec4(_0_unknown);
    return _1_ok && test_int_b() ? colorGreen : colorRed;
}
