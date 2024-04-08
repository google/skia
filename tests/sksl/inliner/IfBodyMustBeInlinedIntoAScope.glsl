
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 a = colorRed;
    vec4 b = colorRed;
    vec4 c = colorRed;
    if (colorGreen.y == 1.0) {
        vec4 _1_x = colorGreen;
        a = _1_x;
    }
    if (colorRed.x == 1.0) {
        vec4 _2_x = colorGreen;
        b = _2_x;
    }
    if (colorGreen.x == 1.0) ; else {
        vec4 _3_x = colorGreen;
        c = _3_x;
    }
    return min(min(a, b), c);
}
