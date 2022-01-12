
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_bi(int one) {
    switch (one) {
        case 0:
            return false;
        case 1:
            return true;
        case 2:
            return false;
        case 3:
            return false;
        case 4:
            return false;
        case 5:
            return false;
        default:
            return false;
    }
}
vec4 main() {
    return test_bi(int(colorGreen.y)) ? colorGreen : colorRed;
}
