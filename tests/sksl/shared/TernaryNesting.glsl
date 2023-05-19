
out vec4 sk_FragColor;
uniform vec4 colorWhite;
vec4 main() {
    vec4 colorBlue = vec4(0.0, 0.0, colorWhite.zw);
    vec4 colorGreen = vec4(0.0, colorWhite.y, 0.0, colorWhite.w);
    vec4 colorRed = vec4(colorWhite.x, 0.0, 0.0, colorWhite.w);
    vec4 result = colorWhite != colorBlue ? (colorGreen == colorRed ? colorRed : colorGreen) : (colorRed != colorGreen ? colorBlue : colorWhite);
    return colorRed == colorBlue ? colorWhite : (colorRed != colorGreen ? result : (colorRed == colorWhite ? colorBlue : colorRed));
}
