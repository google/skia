
uniform vec4 colorGreen;
uniform vec4 colorRed;
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
    bool _4_ok = true;
    return (_4_ok && test_ivec_b()) && test_mat_b() ? colorGreen : colorRed;
}
