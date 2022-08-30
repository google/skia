
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool ifTest_bh4(vec4 color) {
    bool result = bool(color.y);
    return result;
}
vec4 main() {
    if (ifTest_bh4(colorGreen)) return colorGreen; else return colorRed;
}
