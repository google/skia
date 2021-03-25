
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 loopy_h4() {
    for (int x = 0;x <= 4; ++x) {
        return colorGreen;
    }
    return colorRed;
}
vec4 main() {
    return loopy_h4();
}
