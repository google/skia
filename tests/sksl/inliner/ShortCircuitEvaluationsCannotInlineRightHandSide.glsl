
out vec4 sk_FragColor;
uniform vec4 colorWhite;
bool testA_bh4(vec4 v) {
    return bool(v.x);
}
bool testB_bh4(vec4 v) {
    return bool(v.y);
}
vec4 main() {
    vec4 result = vec4(0.0);
    if (bool(colorWhite.x) && testB_bh4(colorWhite)) {
        result.y = 1.0;
    }
    if (bool(colorWhite.y) || testA_bh4(colorWhite)) {
        result.w = 1.0;
    }
    return result;
}
