
out vec4 sk_FragColor;
uniform float unknownInput;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_int() {
    int unknown = int(unknownInput);
    bool ok = true;
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
    return _1_ok && test_int() ? colorGreen : colorRed;
}
