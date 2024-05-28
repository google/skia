
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_fallthrough_groups_bi(int value) {
    bool ok = false;
    switch (value) {
        case -1:
            ok = false;
        case 0:
            return false;
        case 1:
            ok = true;
        case 2:
        case 3:
            break;
        case 4:
            ok = false;
        case 5:
        case 6:
        case 7:
        default:
            break;
    }
    return ok;
}
vec4 main() {
    int x = int(colorGreen.y);
    return switch_fallthrough_groups_bi(x) ? colorGreen : colorRed;
}
