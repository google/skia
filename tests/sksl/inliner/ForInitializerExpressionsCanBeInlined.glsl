
out vec4 sk_FragColor;
uniform vec4 colorGreen;
bool shouldLoop_bh4(vec4 v) {
    return v.x < 0.5;
}
vec4 grow_h4h4(vec4 v) {
    return v + vec4(0.125);
}
vec4 main() {
    for (vec4 color = vec4(0.0625);shouldLoop_bh4(color); color = grow_h4h4(color)) {
    }
    return colorGreen;
}
