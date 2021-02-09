
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test() {
    bool ok = true;
    ok = ok;
    return ok;
}
vec4 main() {
    return test() ? colorGreen : colorRed;
}
