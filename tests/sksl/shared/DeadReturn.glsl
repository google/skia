
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_b() {
    return true;
    return false;
}
vec4 main() {
    return test_b() ? colorGreen : colorRed;
}
