
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 helper_h4();
vec4 main() {
    return helper_h4();
}
vec4 helper_h4() {
    int temp = 1;
    switch (temp) {
        case 0:
            return colorRed;
        case 1:
            return colorGreen;
        case 2:
            return colorRed;
        default:
            return colorRed;
    }
}
