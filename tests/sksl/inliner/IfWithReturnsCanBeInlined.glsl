
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 branchy_h4h4(vec4 c) {
    if (colorGreen == colorRed) return colorRed; else return colorGreen;
}
vec4 main() {
    return branchy_h4h4(colorGreen);
}
