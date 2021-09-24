
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 color;
    switch (int(colorGreen.y)) {
        case 0:
            color = colorRed;
            break;
        case 1:
            color = colorGreen;
            break;
        default:
            color = colorRed;
            break;
    }
    return color;
}
