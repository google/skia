
out vec4 sk_FragColor;
uniform vec4 colorWhite;
vec4 ifElseTest_h4h4h4h4(vec4 colorBlue, vec4 colorGreen, vec4 colorRed) {
    vec4 result = vec4(0.0);
    if (colorWhite != colorBlue) {
        if (colorGreen == colorRed) {
            result = colorRed;
        } else {
            result = colorGreen;
        }
    } else {
        if (colorRed != colorGreen) {
            result = colorBlue;
        } else {
            result = colorWhite;
        }
    }
    if (colorRed == colorBlue) {
        return colorWhite;
    }
    if (colorRed != colorGreen) {
        return result;
    }
    if (colorRed == colorWhite) {
        return colorBlue;
    }
    return colorRed;
}
vec4 main() {
    return ifElseTest_h4h4h4h4(vec4(0.0, 0.0, colorWhite.z, 1.0), vec4(0.0, colorWhite.y, 0.0, 1.0), vec4(colorWhite.x, 0.0, 0.0, 1.0));
}
