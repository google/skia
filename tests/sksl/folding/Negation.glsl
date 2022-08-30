
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_fvec_b() {
    const float one = 1.0;
    float two = 2.0;
    bool ok = true;
    ok = ok && -vec4(two) == vec4(-two, vec3(-two));
    ok = ok && vec2(1.0, -2.0) == -vec2(one - two, two);
    return ok;
}
bool test_ivec_b() {
    int one = 1;
    const int two = 2;
    bool ok = true;
    ok = ok && -ivec2(-one, one + one) == -ivec2(one - two, 2);
    return ok;
}
bool test_mat_b() {
    bool ok = true;
    return ok;
}
vec4 main() {
    return (test_fvec_b() && test_ivec_b()) && test_mat_b() ? colorGreen : colorRed;
}
