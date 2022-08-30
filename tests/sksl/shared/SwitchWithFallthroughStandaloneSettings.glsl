
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_fallthrough_bi(int value) {
    bool ok = false;
    switch (value) {
        case 2:
            break;
        case 1:
        case 0:
            ok = true;
            break;
        default:
            break;
    }
    return ok;
}
bool switch_fallthrough_twice_bi(int value) {
    bool ok = false;
    switch (value) {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
            ok = true;
            break;
        default:
            break;
    }
    return ok;
}
vec4 main() {
    int x = int(colorGreen.y);
    return switch_fallthrough_bi(x) && switch_fallthrough_twice_bi(x) ? colorGreen : colorRed;
}
