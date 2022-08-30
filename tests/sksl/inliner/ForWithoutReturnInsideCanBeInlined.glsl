
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 loopy_h4() {
    vec4 result = colorGreen;
    for (int x = 0;x < 4; ++x) {
        result *= colorGreen;
    }
    return result;
}
vec4 main() {
    return loopy_h4();
}
