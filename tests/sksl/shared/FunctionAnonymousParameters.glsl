
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
struct S {
    int i;
};
vec4 fnGreen_h4bf2(bool b, vec2 _skAnonymousParam1) {
    return colorGreen;
}
vec4 fnRed_h4ifS(int _skAnonymousParam0, float f, S _skAnonymousParam2) {
    return colorRed;
}
vec4 main() {
    return bool(colorGreen.y) ? fnGreen_h4bf2(true, coords) : fnRed_h4ifS(123, 3.1400001049041748, S(0));
}
