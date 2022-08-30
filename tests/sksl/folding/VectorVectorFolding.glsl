
out vec4 sk_FragColor;
uniform float unknownInput;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_half_b() {
    float unknown = unknownInput;
    bool ok = true;
    ok = ok && vec4(0.0) / vec4(unknown) == vec4(0.0);
    vec4 val = vec4(unknown);
    val += vec4(1.0);
    val -= vec4(1.0);
    val = val + vec4(1.0);
    val = val - vec4(1.0);
    ok = ok && val == vec4(unknown);
    val *= vec4(2.0);
    val /= vec4(2.0);
    val = val * vec4(2.0);
    val = val / vec4(2.0);
    ok = ok && val == vec4(unknown);
    return ok;
}
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
    return test_half_b() && test_int_b() ? colorGreen : colorRed;
}
