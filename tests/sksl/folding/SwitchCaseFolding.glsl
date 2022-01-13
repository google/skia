
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    vec4 color = colorRed;
    switch (int(colorGreen.y)) {
        case 0:
            break;
        case 1:
            color = colorGreen;
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            break;
        default:
            break;
    }
    return color;
}
