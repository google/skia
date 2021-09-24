
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    switch (int(colorGreen.y)) {
        case 0:
        default:
            return colorGreen;
    }
}
