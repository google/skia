
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool shouldLoop_bh4(vec4 v) {
    return v != colorGreen;
}
vec4 main() {
    vec4 result = colorRed;
    while (shouldLoop_bh4(result)) {
        result = colorGreen;
    }
    return result;
}
