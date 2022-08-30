
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 ifBody_h4() {
    vec4 x = colorGreen;
    return x;
}
vec4 main() {
    vec4 c = colorRed;
    if (colorGreen.y == 1.0) c = ifBody_h4();
    return c;
}
