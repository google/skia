
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool shouldLoop_bh4(vec4 value) {
    return value != colorGreen;
}
vec4 main() {
    vec4 result = colorRed;
    do {
        result = colorGreen;
    } while (shouldLoop_bh4(result));
    return result;
}
