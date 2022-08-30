
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_b() {
    bool ok = true;
    return ok;
}
vec4 main() {
    return test_b() ? colorGreen : colorRed;
}
