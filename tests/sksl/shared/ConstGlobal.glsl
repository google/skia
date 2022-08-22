
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
const int SEVEN = 7;
const int TEN = 10;
bool verify_const_globals_bii(int seven, int ten) {
    return seven == 7 && ten == 10;
}
vec4 main() {
    return verify_const_globals_bii(SEVEN, TEN) ? colorGreen : colorRed;
}
