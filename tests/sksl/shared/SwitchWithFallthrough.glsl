
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_fallthrough_bi(int value) {
    switch (value) {
        case 2:
            return false;
        case 1:
        case 0:
            return true;
        default:
            return false;
    }
}
bool switch_fallthrough_twice_bi(int value) {
    switch (value) {
        case 0:
            return false;
        case 1:
        case 2:
        case 3:
            return true;
        default:
            return false;
    }
}
vec4 main() {
    int x = int(colorGreen.y);
    return switch_fallthrough_bi(x) && switch_fallthrough_twice_bi(x) ? colorGreen : colorRed;
}
