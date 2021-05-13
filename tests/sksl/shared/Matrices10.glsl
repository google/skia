
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half_b() {
    bool ok = true;
    vec3 v1 = mat3(2.0) * vec3(3.0);
    ok = ok && v1 == vec3(6.0);
    vec3 v2 = vec3(3.0) * mat3(3.0);
    ok = ok && v2 == vec3(9.0);
    return ok;
}
vec4 main() {
    bool _0_ok = true;
    vec3 _1_v1 = mat3(2.0) * vec3(3.0);
    _0_ok = _0_ok && _1_v1 == vec3(6.0);
    vec3 _2_v2 = vec3(3.0) * mat3(3.0);
    _0_ok = _0_ok && _2_v2 == vec3(9.0);
    return _0_ok && test_half_b() ? colorGreen : colorRed;
}
