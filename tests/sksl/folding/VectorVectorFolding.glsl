
uniform float unknownInput;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_int_b() {
    int unknown = int(unknownInput);
    bool ok = true;
    ok = ok && ivec4(0) / ivec4(unknown) == ivec4(0);
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
    _1_ok = _1_ok && vec4(0.0) / vec4(_0_unknown) == vec4(0.0);
    vec4 _2_val = vec4(_0_unknown);
    _2_val += vec4(1.0);
    _2_val -= vec4(1.0);
    _2_val = _2_val + vec4(1.0);
    _2_val = _2_val - vec4(1.0);
    _1_ok = _1_ok && _2_val == vec4(_0_unknown);
    _2_val *= vec4(2.0);
    _2_val *= vec4(0.5);
    _2_val = _2_val * vec4(2.0);
    _2_val = _2_val * vec4(0.5);
    _1_ok = _1_ok && _2_val == vec4(_0_unknown);
    return _1_ok && test_int_b() ? colorGreen : colorRed;
}
