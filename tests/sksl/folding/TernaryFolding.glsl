
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool do_side_effect_bb(out bool x) {
    x = true;
    return false;
}
vec4 main() {
    bool ok = true;
    vec4 green = colorGreen;
    vec4 red = colorRed;
    bool param = false;
    bool call = (do_side_effect_bb(param), true);
    return (ok && param) && call ? green : red;
}
