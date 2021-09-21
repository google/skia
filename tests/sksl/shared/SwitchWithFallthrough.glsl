
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
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
    bool _0_ok = false;
    switch (x) {
        case 2:
            break;
        case 1:
        case 0:
            _0_ok = true;
            break;
        default:
            break;
    }
    return _0_ok && switch_fallthrough_twice_bi(x) ? colorGreen : colorRed;
}
