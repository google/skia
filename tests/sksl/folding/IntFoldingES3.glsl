
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_b() {
    bool ok = true;
    int x = 14;
    ok = ok && x == 14;
    x = 6;
    ok = ok && x == 6;
    x = 5;
    ok = ok && x == 5;
    x = 16;
    ok = ok && x == 16;
    x = -8;
    ok = ok && x == -8;
    x = 32;
    ok = ok && x == 32;
    x = 33;
    ok = ok && x == 33;
    return ok;
}
vec4 main() {
    return test_b() ? colorGreen : colorRed;
}
