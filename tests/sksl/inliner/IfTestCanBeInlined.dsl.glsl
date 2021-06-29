
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool _0_result = bool(colorGreen.y);
    if (_0_result) return colorGreen; else return colorRed;
}
