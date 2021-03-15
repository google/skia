
out vec4 sk_FragColor;
uniform float unknownInput;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_half() {
    float unknown = unknownInput;
    bool ok = true;
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
    return test_half() && test_int() ? colorGreen : colorRed;
}
