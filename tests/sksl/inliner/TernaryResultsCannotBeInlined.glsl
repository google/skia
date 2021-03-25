
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 trueSide_h4h4(vec4 v) {
    return vec4(v.x, 1.0, v.zw);
}
vec4 falseSide_h4h4(vec4 v) {
    return vec4(v.x, 0.0, v.zw);
}
vec4 main() {
    return bool(colorGreen.y) ? trueSide_h4h4(colorGreen) : falseSide_h4h4(colorRed);
}
