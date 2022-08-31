
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 _0_branchy;
    if (colorGreen == colorRed) _0_branchy = colorRed; else _0_branchy = colorGreen;
    return _0_branchy;
}
