
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 elseBody_h4() {
    vec4 x = colorGreen;
    return x;
}
vec4 main() {
    vec4 c = colorRed;
    if (colorGreen.y == 0.0) ; else c = elseBody_h4();
    return c;
}
