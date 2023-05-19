
out vec4 sk_FragColor;
uniform vec4 colorWhite;
bool IsEqual_bh4h4(vec4 x, vec4 y) {
    return x == y;
}
vec4 main() {
    vec4 colorBlue = vec4(0.0, 0.0, colorWhite.zw);
    vec4 colorGreen = vec4(0.0, colorWhite.y, 0.0, colorWhite.w);
    vec4 colorRed = vec4(colorWhite.x, 0.0, 0.0, colorWhite.w);
    vec4 result = !IsEqual_bh4h4(colorWhite, colorBlue) ? (IsEqual_bh4h4(colorGreen, colorRed) ? colorRed : colorGreen) : (!IsEqual_bh4h4(colorRed, colorGreen) ? colorBlue : colorWhite);
    return IsEqual_bh4h4(colorRed, colorBlue) ? colorWhite : (!IsEqual_bh4h4(colorRed, colorGreen) ? result : (IsEqual_bh4h4(colorRed, colorWhite) ? colorBlue : colorRed));
}
